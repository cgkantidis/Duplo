#include "Block.h"
#include "ConsoleExporter.h"
#include "FileExporter.h"

#include <iostream>

ConsoleExporter::ConsoleExporter(const Options& options)
    : FileExporter(options, true) {
}

void ConsoleExporter::LogMessage(const std::string& message) {
    Log() << message << std::flush;
}

void ConsoleExporter::WriteHeader() {}
void ConsoleExporter::WriteFooter(
    const Options& options,
    int files,
    long locsTotal,
    unsigned tot_dup_blocks,
    unsigned tot_dup_lines) {
    Out()
        << "Configuration:"
        << "\n  Number of files: "
        << files
        << "\n  Minimal block size: "
        << options.GetMinBlockSize()
        << "\n  Minimal characters in line: "
        << options.GetMinChars()
        << "\n  Ignore preprocessor directives: "
        << options.GetIgnorePrepStuff()
        << "\n  Ignore same filenames: "
        << options.GetIgnoreSameFilename()
        << "\n\nResults:"
        << "\n  Lines of code: "
        << locsTotal
        << "\n  Duplicate lines of code: "
        << tot_dup_lines
        << "\n  Total "
        << tot_dup_blocks
        << " duplicate block(s) found.\n\n";
}

void ConsoleExporter::ReportSeq(
    int line1,
    int line2,
    int count,
    const SourceFile& source1,
    const SourceFile& source2) {
    Out()
        << source1.GetFilename()
        << "(" << source1.GetLine(line1).GetLineNumber() << ")\n";
    Out()
        << source2.GetFilename()
        << "(" << source2.GetLine(line2).GetLineNumber() << ")\n";
    for (int j = 0; j < count; j++) {
        Out() << source1.GetLine(j + line1).GetLine() << '\n';
    }

    Out() << '\n';
}
