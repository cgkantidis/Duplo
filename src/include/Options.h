#ifndef _OPTIONS_H_
#define _OPTIONS_H_

#include <string>

class Options {
    unsigned m_minChars;
    bool m_ignorePrepStuff;
    unsigned m_minBlockSize;
    unsigned m_blockPercentThreshold;
    unsigned m_filesToCheck;
    unsigned m_numThreads;
    bool m_outputXml;
    bool m_outputJSON;
    bool m_ignoreSameFilename;
    std::string m_listFilename;
    std::string m_outputFilename;

public:
    Options(
        unsigned minChars,
        bool ignorePrepStuff,
        unsigned minBlockSize,
        unsigned blockPercentThreshold,
        unsigned m_filesToCheck,
        unsigned numThreads,
        bool outputXml,
        bool outputJSON,
        bool ignoreSameFilename,
        const std::string& listFilename,
        const std::string& outputFilename
    );

    bool GetIgnoreSameFilename() const;
    const std::string& GetListFilename() const;
    const std::string& GetOutputFilename() const;
    bool GetOutputXml() const;
    bool GetOutputJSON() const;
    unsigned GetMinChars() const;
    bool GetIgnorePrepStuff() const;
    unsigned GetMinBlockSize() const;
    unsigned GetBlockPercentThreshold() const;
    size_t GetFilesToCheck() const;
    [[nodiscard]] unsigned GetNumThreads() const {
        return m_numThreads;
    }
};

#endif
