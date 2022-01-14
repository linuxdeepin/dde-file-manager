/**
 * @brief     DOC files into HTML сonverter
 * @package   doc
 * @file      doc.cpp
 * @author    dmryutov (dmryutov@gmail.com)
 * @copyright rembish (https://github.com/rembish/TextAtAnyCost)
 * @date      03.08.2017 -- 29.01.2018
 */
#include <regex>

#include "tools.hpp"

#include "doc.hpp"


namespace doc {

const std::regex OBJECT1_MASK("HYPER13 *(INCLUDEPICTURE|HTMLCONTROL)(.*)HYPER15", std::regex::icase);
const std::regex OBJECT2_MASK("HYPER13(.*)HYPER14(.*)HYPER15", std::regex::icase);

// public:
Doc::Doc(const std::string& fileName)
    : FileExtension(fileName), Cfb(fileName) {}

int Doc::convert(bool addStyle, bool extractImages, char mergingMode) {
    Cfb::parse();
    // DOC needs two streams for reading DOC - `WordDocument` and `0Table` or `1Table`, depending
    // on situation. Find `WordDocument` - it contains pieces of text
    std::string wDocumentStream = getStream("WordDocument");
    if (wDocumentStream.empty())
        return 1;
    // Get data from FIB (File Information Block) - special block in the begining of `WordDocument`
    unsigned short fib = readByte<unsigned short>(wDocumentStream, 0x000A, 2);
    // Decide which table should be read - `0Table` или `1Table`
    bool whichTableStream = ((fib & 0x0200) == 0x0200);

    // Get offset and size of CLX in table stream
    int clxPos  = readByte<int>(wDocumentStream, 0x01A2, 4);
    int clxSize = readByte<int>(wDocumentStream, 0x01A6, 4);

    // Read several values to separate positions from sizes in CLX
    int ccpText    = readByte<int>(wDocumentStream, 0x004C, 4);
    int ccpFtn     = readByte<int>(wDocumentStream, 0x0050, 4);
    int ccpHdd     = readByte<int>(wDocumentStream, 0x0054, 4);
    int ccpMcr     = readByte<int>(wDocumentStream, 0x0058, 4);
    int ccpAtn     = readByte<int>(wDocumentStream, 0x005C, 4);
    int ccpEdn     = readByte<int>(wDocumentStream, 0x0060, 4);
    int ccpTxbx    = readByte<int>(wDocumentStream, 0x0064, 4);
    int ccpHdrTxbx = readByte<int>(wDocumentStream, 0x0068, 4);
    // Calculate value of last CP (Character Position)
    int lastCP = ccpFtn + ccpHdd + ccpMcr + ccpAtn + ccpEdn + ccpTxbx + ccpHdrTxbx;
    lastCP    += (lastCP != 0) + ccpText;

    // Find necessary table
    std::string tableStream = getStream(std::to_string(whichTableStream) + "Table");
    if (tableStream.empty() || tableStream.size() < clxPos + clxSize)
        return 2;
    Cfb::clear();

    // Find CLX
    std::string clx = tableStream.substr(clxPos, clxSize);
    // Find in CLX a piece with offsets and sizes of pieces of text
    size_t lcbPieceTable = 0;
    std::string pieceTable;

    // Find possible start of `pieceTable` (it must start at 0x02), then read next 4 bytes
    // (size of pieceTable)
    size_t from = 0;
    size_t pos;
    // Find 0x02 from current offset in CLX
    while ((pos = clx.find_first_of((char)0x02, from)) != std::string::npos) {
        // Find `pieceTable` and its size
        pieceTable    = clx.substr(pos + 5);
        lcbPieceTable = readByte<size_t>(clx, pos + 1, 4);
        // If actual size differs from desired size then find again
        if (pieceTable.size() != lcbPieceTable)
            from = pos + 1;
        else
            break;
    }

    // Fill Character Positions array until we find last CP
    std::vector<int> cp;
    pos = 0;
    for (int i = readByte<int>(pieceTable, pos, 4); i != lastCP;
         pos += 4, i = readByte<int>(pieceTable, pos, 4))
    {
        cp.push_back(i);
    }
    cp.push_back(lastCP);
    // The remainder is PCD (Piece Descriptors)
    std::vector<std::string> pcd;
    int j = 1;
    std::string s;
    for (const auto& c : pieceTable.substr(pos + 4)) {
        s += c;
        if (j == 8) {
            pcd.push_back(s);
            s.clear();
            j = 0;
        }
        j++;
    }
    if (!s.empty())
        pcd.push_back(s);

    // Get text from file. Iterate through PCD
    std::string text;
    for (size_t i = 0; i < pcd.size(); ++i) {
        // Get size of piece of text
        int lcb = cp[i + 1] - cp[i];

        // Get word with offset and compression flag
        int fcValue = readByte<int>(pcd[i], 2, 4);
        // Decide if encoding is ANSI or Unicode
        bool isANSI = (fcValue & 0x40000000) == 0x40000000;
        // The remainder without header is offset
        int fc = fcValue & 0x3FFFFFFF;
        // If ANSI then start twice earlier
        if (isANSI)
            fc /= 2;
        // If Unicode then read twice size
        else
            lcb *= 2;

        std::string part = wDocumentStream.substr(fc, lcb);
        if (!isANSI)
            part = unicodeToUtf8(part);  // tools::decode(part, "UTF-16LE");
        text += part;
    }

    // Delete links and embedded objects
    text = text.substr(0, 100000);
    text = regex_replace(text, OBJECT1_MASK, "");
    text = regex_replace(text, OBJECT2_MASK, "$2");

    // Separate pargraphs and add them to HTML tags
    for (const auto& line : tools::explode(text, "\n\r")) {
        if (line.empty()) {
            m_text += "\u00A0";
        } else {
            m_text += line + '\n';
        }
    }
    return 0;
}

}  // End namespace
