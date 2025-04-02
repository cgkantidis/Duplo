#include "Block.h"
#include "XmlExporter.h"
#include "Utils.h"

#include <iostream>

XmlExporter::XmlExporter(const Options& options)
    : FileExporter(options, false) {
}

void XmlExporter::LogMessage(const std::string& message) {
    Log() << message << std::flush;
}

void XmlExporter::WriteHeader() {
    Out()
        << "<?xml version=\"1.0\"?>\n"
        << "<duplo>\n";
}

void XmlExporter::WriteFooter(
    const Options& /*options*/,
    int /*files*/,
    long /*locsTotal*/,
    unsigned /*tot_dup_blocks*/,
    unsigned /*tot_dup_lines*/) {
    Out()
        << "</duplo>\n";
}

void XmlExporter::ReportSeq(
    int line1,
    int line2,
    int count,
    const SourceFile& source1,
    const SourceFile& source2) {
    Out()
        << "    <set LineCount=\"" << count << "\">\n";
    int startLineNumber1 = source1.GetLine(line1).GetLineNumber();
    int endLineNumber1 = source1.GetLine(line1 + count - 1).GetLineNumber();
    Out()
        << "        <block SourceFile=\"" << source1.GetFilename()
        << "\" StartLineNumber=\"" << startLineNumber1
        << "\" EndLineNumber=\"" << endLineNumber1 << "\"/>\n";
    int startLineNumber2 = source2.GetLine(line2).GetLineNumber();
    int endLineNumber2 = source2.GetLine(line2 + count - 1).GetLineNumber();
    Out()
        << "        <block SourceFile=\"" << source2.GetFilename()
        << "\" StartLineNumber=\"" << startLineNumber2
        << "\" EndLineNumber=\"" << endLineNumber2 << "\"/>\n";
    Out()
        << "        <lines xml:space=\"preserve\">\n";
    for (int j = 0; j < count; j++) {
        // replace various characters/ strings so that it doesn't upset the XML parser
        std::string tmpstr = source1.GetLine(j + line1).GetLine();

        // " --> '
        StringUtil::StrSub(tmpstr, "\'", "\"", -1);

        // & --> &amp;
        StringUtil::StrSub(tmpstr, "&amp;", "&", -1);

        // < --> &lt;
        StringUtil::StrSub(tmpstr, "&lt;", "<", -1);

        // > --> &gt;
        StringUtil::StrSub(tmpstr, "&gt;", ">", -1);

        Out() << "            <line Text=\"" << tmpstr << "\"/>\n";
    }

    Out() << "        </lines>\n";
    Out() << "    </set>\n";
}
