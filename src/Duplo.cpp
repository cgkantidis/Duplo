#include "Duplo.h"
#include "IExporter.h"
#include "Options.h"
#include "ProcessResult.h"
#include "SourceFile.h"
#include "SourceLine.h"
#include "Utils.h"

#include <algorithm>
#include <cstring>
#include <ctime>
#include <cmath>
#include <format>
#include <iostream>
#include <mutex>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <thread>
#include <ranges>

#include <BS_thread_pool.hpp>

typedef std::tuple<unsigned, std::string> FileLength;
typedef const std::string* StringPtr;
using HashToFiles = std::unordered_map<unsigned long, std::vector<StringPtr>>;
using thread_pool = BS::thread_pool<>;

static constexpr std::size_t TOP_N_LONGEST = 10;

namespace {
    void printLongestFiles(std::ostringstream &oss, std::vector<SourceFile> &sourceFiles, std::size_t top_n) {
        std::sort(std::begin(sourceFiles),
                  std::end(sourceFiles),
                  [](SourceFile const& l, SourceFile const& r) {
            return l.GetNumOfLines() > r.GetNumOfLines();
        });
        std::for_each_n(std::begin(sourceFiles), top_n, [&oss](SourceFile const& file) {
            oss << file.GetNumOfLines() << ": " << file.GetFilename() << '\n';
        });
    }

    std::tuple<std::vector<SourceFile>, std::vector<bool>, unsigned, unsigned, std::size_t> LoadSourceFiles(
        const std::vector<std::string>& lines,
        unsigned minChars,
        bool ignorePrepStuff,
        IExporterPtr exporter) {

        std::vector<SourceFile> sourceFiles;
        std::vector<bool> matrix;
        size_t maxLinesPerFile = 0;
        int files = 0;
        unsigned long locsTotal = 0;

        // Create vector with all source files
        for (size_t i = 0; i < lines.size(); i++) {
            if (lines[i].size() > 5) {
                SourceFile sourceFile(lines[i], minChars, ignorePrepStuff);
                size_t numLines = sourceFile.GetNumOfLines();
                if (numLines > 0) {
                    files++;
                    sourceFiles.push_back(std::move(sourceFile));
                    locsTotal += numLines;
                    if (maxLinesPerFile < numLines) {
                        maxLinesPerFile = numLines;
                    }
                }
            }
        }

        if (maxLinesPerFile * maxLinesPerFile > matrix.max_size()) {
            std::ostringstream stream;
            stream
                << "Some files have too many lines. You can have files with approximately "
                << std::sqrt(matrix.max_size())
                << " lines at most." << std::endl
                << "Longest files:" << std::endl;
            printLongestFiles(stream, sourceFiles, TOP_N_LONGEST);
            throw std::runtime_error(stream.str().c_str());
        }

        exporter->LogMessage(std::format("{} done.\n\n", lines.size()));

        // Generate matrix large enough for all files
        try {
            matrix.resize(maxLinesPerFile * maxLinesPerFile);
        }
        catch (const std::bad_alloc& ex) {
            std::ostringstream stream;
            stream
                << ex.what() << std::endl
                << "Longest files:" << std::endl;
            printLongestFiles(stream, sourceFiles, TOP_N_LONGEST);
            throw std::runtime_error(stream.str().c_str());
        }

        return std::tuple(std::move(sourceFiles), matrix, files, locsTotal, maxLinesPerFile);
    }

    ProcessResult Process(
        const SourceFile& source1,
        const SourceFile& source2,
        std::vector<bool>& matrix,
        const Options& options,
        IExporterPtr exporter,
        std::mutex &exporter_mtx) {
        size_t m = source1.GetNumOfLines();
        size_t n = source2.GetNumOfLines();

        // Reset matrix data
        std::fill(std::begin(matrix), std::begin(matrix) + m * n, false);

        // Compute matrix
        for (size_t y = 0; y < m; y++) {
            auto& line = source1.GetLine(y);
            for (size_t x = 0; x < n; x++) {
                if (line == source2.GetLine(x)) {
                    matrix[x + n * y] = true;
                }
            }
        }

        // support reporting filtering by both:
        // - "lines of code duplicated", &
        // - "percentage of file duplicated"
        unsigned lMinBlockSize = std::max(
            (size_t)options.GetMinBlockSize(),
            std::min(
                (size_t)options.GetMinBlockSize(),
                (std::max(n, m) * 100) / options.GetBlockPercentThreshold()));

        unsigned blocks = 0;
        unsigned duplicateLines = 0;

        // make curried function for invoking ReportSeq
        auto reportSeq = [&source1, &source2, &exporter, &exporter_mtx](int line1, int line2, int count) {
            std::scoped_lock sl(exporter_mtx);
            exporter->ReportSeq(
                line1,
                line2,
                count,
                source1,
                source2);
        };

        // Scan vertical part
        for (size_t y = 0; y < m; y++) {
            unsigned seqLen = 0;
            size_t maxX = std::min(n, m - y);
            for (size_t x = 0; x < maxX; x++) {
                if (matrix[x + n * (y + x)]) {
                    seqLen++;
                } else {
                    if (seqLen >= lMinBlockSize) {
                        int line1 = y + x - seqLen;
                        int line2 = x - seqLen;
                        if (line1 != line2 || source1 != source2) {
                            reportSeq(line1, line2, seqLen);
                            duplicateLines += seqLen;
                            blocks++;
                        }
                    }

                    seqLen = 0;
                }
            }

            if (seqLen >= lMinBlockSize) {
                int line1 = m - seqLen;
                int line2 = n - seqLen;
                if (line1 != line2 || source1 != source2) {
                    reportSeq(line1, line2, seqLen);
                    duplicateLines += seqLen;
                    blocks++;
                }
            }
        }

        if (source1 != source2) {
            // Scan horizontal part
            for (size_t x = 1; x < n; x++) {
                unsigned seqLen = 0;
                size_t maxY = std::min(m, n - x);
                for (size_t y = 0; y < maxY; y++) {
                    if (matrix[x + y + n * y]) {
                        seqLen++;
                    } else {
                        if (seqLen >= lMinBlockSize) {
                            reportSeq(y - seqLen, x + y - seqLen, seqLen);
                            duplicateLines += seqLen;
                            blocks++;
                        }
                        seqLen = 0;
                    }
                }

                if (seqLen >= lMinBlockSize) {
                    reportSeq(m - seqLen, n - seqLen, seqLen);
                    duplicateLines += seqLen;
                    blocks++;
                }
            }
        }

        return ProcessResult(blocks, duplicateLines);
    }
}

void task1(thread_pool &pool,
        std::vector<SourceFile>::iterator l_it,
        std::vector<SourceFile>::iterator r_end,
        HashToFiles const &hashToFiles,
        Options const &options,
        IExporterPtr &exporter,
        ProcessResult &processResultTotal,
        std::mutex &log_mtx,
        std::size_t &progress,
        std::size_t &num_files,
        std::unordered_map<std::thread::id, std::vector<bool>> &matrices) {
    // get matching files
    std::unordered_set<StringPtr> matchingFiles;
    for (std::size_t k = 0; k < l_it->GetNumOfLines(); k++) {
        auto hash = l_it->GetLine(k).GetHash();
        const auto& filenames = hashToFiles.find(hash)->second;
        matchingFiles.insert(filenames.begin(), filenames.end());
    }

    std::vector<bool> &matrix = matrices.find(std::this_thread::get_id())->second;

    ProcessResult processResult =
        Process(
            *l_it,
            *l_it,
            matrix,
            options,
            exporter,
            log_mtx);

    // files to compare are those that have matching lines
    for (auto r_it = std::next(l_it); r_it != r_end; ++r_it) {
        if (options.GetIgnoreSameFilename() && StringUtil::IsSameFilename(*l_it, *r_it)) {
            continue;
        }
        if (matchingFiles.find(&r_it->GetFilename()) == matchingFiles.end()) {
            continue;
        }
        processResult
            << Process(
                   *l_it,
                   *r_it,
                   matrix,
                   options,
                   exporter,
                   log_mtx);
    }

    {
        std::scoped_lock sl(log_mtx);
        ++progress;
        if (processResult.Blocks() > 0) {
            exporter->LogMessage(std::format("{}/{} ({:.1f}%) {} found: {} block(s)\n", progress, num_files, 100.0 * progress / num_files, l_it->GetFilename(), processResult.Blocks()));
        } else {
            exporter->LogMessage(std::format("{}/{} ({:.1f}%) {} nothing found.\n", progress, num_files, 100.0 * progress / num_files, l_it->GetFilename()));
        }

        processResultTotal << processResult;
    }
}

int Duplo::Run(const Options& options) {

    IExporterPtr exporter = IExporter::CreateExporter(options);
    exporter->LogMessage("Loading and hashing files ... ");

    exporter->WriteHeader();

    auto lines = FileSystem::LoadFileList(options.GetListFilename());
    auto [sourceFiles, matrix, files, locsTotal, max_lines] = LoadSourceFiles(
        lines,
        options.GetMinChars(),
        options.GetIgnorePrepStuff(),
        exporter);

    std::size_t progress = 0;
    std::size_t num_files = sourceFiles.size();
    auto end_it = sourceFiles.end();
    if (options.GetFilesToCheck() > 0 && options.GetFilesToCheck() < sourceFiles.size()) {
        end_it = std::next(sourceFiles.begin(), options.GetFilesToCheck());
        num_files = options.GetFilesToCheck();
    }

    // hash maps
    HashToFiles hashToFiles;
    for (const auto& s : sourceFiles) {
        for (size_t i = 0; i < s.GetNumOfLines(); i++) {
            hashToFiles[s.GetLine(i).GetHash()].push_back(&s.GetFilename());
        }
    }

    thread_pool pool(options.getNumThreads());
    std::unordered_map<std::thread::id, std::vector<bool>> matrices;
    for (auto const &thread_id : pool.get_thread_ids()) {
        matrices[thread_id] = std::vector<bool>(max_lines * max_lines);
    }

    // Compare each file with each other
    ProcessResult processResultTotal;
    std::mutex log_mtx;
    for (auto l_it = sourceFiles.begin(), l_end = std::prev(end_it); l_it != l_end; ++l_it) {
        pool.detach_task([&pool, l_it, end_it, &hashToFiles, &options, &exporter, &processResultTotal, &log_mtx, &progress, &num_files, &matrices]{
            task1(pool, l_it, end_it, hashToFiles, options, exporter, processResultTotal, log_mtx, progress, num_files, matrices);
        });
    }
    pool.wait();

    exporter->WriteFooter(options, files, locsTotal, processResultTotal);

    return processResultTotal.Blocks() > 0
               ? EXIT_FAILURE
               : EXIT_SUCCESS;
}

