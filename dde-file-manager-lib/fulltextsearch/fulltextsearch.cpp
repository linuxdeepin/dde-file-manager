/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////
#include "fulltextsearch.h"
#include <QString>
#include <targetver.h>
#include <iostream>
#include "LuceneHeaders.h"
#include "FileUtils.h"
#include "MiscUtils.h"
#include "StringUtils.h"
#include <boost/algorithm/string.hpp>
#include "FilterIndexReader.h"
#include <string>
#include <locale>
#include <codecvt>
#include <fstream>
#include "misc.h"
#include "doctotext_unzip.h"
#include "metadata.h"
#include "plain_text_extractor.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <limits.h>
#include<unistd.h>
#include<dirent.h>
#include<sys/types.h>
#include <locale.h>
#include <QtConcurrentRun>
#include <QStringList>
#include "dfmapplication.h"

#include <QStandardPaths>
#include <QApplication>
DFM_BEGIN_NAMESPACE


#define SEARCH_RESULT_NUM   100000
using namespace Lucene;

Q_GLOBAL_STATIC(QStringList, searchResult)

std::string wstring2utf8string(const std::wstring &str)
{
    static std::wstring_convert<std::codecvt_utf8<wchar_t> > strCnv;
    return strCnv.to_bytes(str);
}

std::wstring utf8string2wstring(const std::string &str)
{
    static std::wstring_convert< std::codecvt_utf8<wchar_t> > strCnv;
    return strCnv.from_bytes(str);
}
int32_t docNumber = 0;
std::string get_document(std::string input)
{
    bool extract_metadata = false;
    std::string text = "";
    FormattingStyle options;
    options.table_style = TABLE_STYLE_TABLE_LOOK;
    options.list_style.setPrefix(" * ");
    options.url_style = URL_STYLE_UNDERSCORED;
    XmlParseMode mode = PARSE_XML;
    clock_t start, end;
    start = clock();           /*记录起始时间*/
    printf("filename:%s\n", input.c_str());

    size_t last = input.find_last_of("/");
    std::string name = input.substr(last + 1);
    std::string dir = input.substr(0, last);
    std::string ext = input.substr(input.find_last_of(".") + 1);

    PlainTextExtractor::ParserType parser_type = PlainTextExtractor::PARSER_AUTO;
    try {
        if (ext == "rtf")
            parser_type = PlainTextExtractor::PARSER_RTF;
        else if (ext == "odt" || ext == "ods" || ext == "odp" || ext == "odg" || ext == "docx" || ext == "xlsx" || ext == "pptx" || ext == "ppsx")
            parser_type = PlainTextExtractor::PARSER_ODF_OOXML;
        else if (ext == "xls")
            parser_type = PlainTextExtractor::PARSER_XLS;
        else if (ext == "xlsb")
            parser_type = PlainTextExtractor::PARSER_XLSB;
        else if (ext == "doc" || ext == "dot" || ext == "wps")
            parser_type = PlainTextExtractor::PARSER_DOC;
        else if (ext == "ppt" || ext == "pps")
            parser_type = PlainTextExtractor::PARSER_PPT;
//        else if (ext == "htm" || ext == "html")
//            parser_type = PlainTextExtractor::PARSER_HTML;
        else if (ext == "pdf")
            parser_type = PlainTextExtractor::PARSER_PDF;
        else if (ext == "txt" || ext == "text")
            parser_type = PlainTextExtractor::PARSER_TXT;
//            extract_metadata = true;
        else {
            std::cout << "Unsupported file extension: " << ext << std::endl;
            return "";
        }
        std::cout << "Conversion complete: " << input << std::endl;
    } catch (...) {
        std::cerr << "Error: " << input << std::endl;
    }
    //创建文件解析器
    PlainTextExtractor extractor(parser_type);

    //设置 xml 格式解析模式
    if (mode != PARSE_XML) {
        extractor.setXmlParseMode(mode);
    }

    //
    extractor.setFormattingStyle(options);

    if (extract_metadata) {
        std::ifstream in(input.c_str());
        if (!in.is_open()) {
            printf("open src File  Error opening file\n");;
            return "";
        }
        char *buffer = new char[1024];
        while (!in.eof()) {
            in.read(buffer, 1024);
            text.append(buffer);
        }
        in.close();
    } else {
        //解析文件内容,
        /*
         * 原因是解析出来的内容和 tests目录下面 *.out 中的内容不相符，
         * 例如: 解析 tests/1.doc.out 最终结果的内容就是现在下面 printf 所打印输出的内容格式，
         * 从可以 tests/Makefile 中看到，在测试的过程中是通过 diff 命令比较，也就是通过比较 1.doc
         * 的解析结果内容和 1.doc.out 内容的是否相同来判断程序是否正常运行的
         */
        if (!extractor.processFile(input, text)) {
            std::cerr << "Error processing file " << input << ".\n";
            return "";
        }
        std::vector<Link> links;
        extractor.getParsedLinks(links);
        if (links.size() > 0) {
            printf("parsed links:\n");
            for (size_t i = 0; i < links.size(); ++i) {
                printf("%s @ index = %d length = %d\n", links[i].getLinkUrl(), links[i].getLinkTextPosition(), strlen(links[i].getLinkText()));
            }
        }
        //解析附件，这个现在只有 eml 类型的实现
        std::vector<Attachment> attachments;
        extractor.getAttachments(attachments);
        if (attachments.size() > 0) {
            printf("parsed attachments:\n");
            for (size_t i = 0; i < attachments.size(); ++i) {
                printf("\nname: %s\n", attachments[i].filename());
                std::map<std::string, Variant> variables = attachments[i].getFields();
                for (std::map<std::string, Variant>::iterator it = variables.begin(); it != variables.end(); ++it) {
                    if (it->first != "Content-ID") {
                        printf("field: %s, value: %s\n", it->first.c_str(), it->second.getString());
                    }
                }
            }
        }
    }
    end = clock();           /*记录结束时间*/
    {
        double seconds  = (double)(end - start) / CLOCKS_PER_SEC;
        fprintf(stderr, "Use time is: %.8f s\n", seconds);
    }
    return text.c_str();
}
std::string readtxt(const String &docFile)
{
    std::string filename = StringUtils::toUTF8(docFile);
    std::ifstream in(filename.c_str());
    std::string contents = "";
    if (!in.is_open()) {
        printf("open src File  Error opening file\n");;
        return contents;
    }
    char *buffer = new char[1024];
    while (!in.eof()) {
        in.read(buffer, 1024);
        contents.append(buffer);
    }
    return  contents;
}
DocumentPtr TXTFileDocument(const String &docFile)
{
    DocumentPtr doc = newLucene<Document>();

    // Add the path of the file as a field named "path".  Use a field that is indexed (ie. searchable), but
    // don't tokenize the field into words.
    doc->add(newLucene<Field>(L"path", docFile, Field::STORE_YES, Field::INDEX_NOT_ANALYZED));

    // Add the last modified date of the file a field named "modified".  Use a field that is indexed (ie. searchable),
    // but don't tokenize the field into words.
    doc->add(newLucene<Field>(L"modified", DateTools::timeToString(FileUtils::fileModified(docFile), DateTools::RESOLUTION_MINUTE),
                              Field::STORE_YES, Field::INDEX_NOT_ANALYZED));

    // Add the contents of the file to a field named "contents".  Specify a Reader, so that the text of the file is
    // tokenized and indexed, but not stored.  Note that FileReader expects the file to be in the system's default
    // encoding.  If that's not the case searching for special characters will fail.
//    std::wcout << "-----------this is the txt--------------\r\n" << std::endl;
    std::string filename = StringUtils::toUTF8(docFile);
    std::ifstream in(filename.c_str());
    std::string contents;
    if (!in.is_open()) {
        printf("open src File  Error opening file\n");;
        return doc;
    }
    char *buffer = new char[1024];
    while (!in.eof()) {
        in.read(buffer, 1024);
        contents.append(buffer);
    }
//    std::wcout << "txt contents:" << StringUtils::toUnicode(contents) << std::endl;
    in.close();
    doc->add(newLucene<Field>(L"contents", StringUtils::toUnicode(contents), Field::STORE_YES, Field::INDEX_ANALYZED));
    //    doc->add(newLucene<Field>(L"contents", newLucene<FileReader>(docFile)));

    return doc;
}
//先将字段加入到文档,在将文档加入到IndexWriter中
DocumentPtr fileDocument(const String &filename)
{
    DocumentPtr doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"path", filename, Field::STORE_YES, Field::INDEX_ANALYZED));
    std::string contents = get_document(StringUtils::toUTF8(filename));
    doc->add(newLucene<Field>(L"contents", StringUtils::toUnicode(contents), Field::STORE_YES, Field::INDEX_ANALYZED));
    return doc;

}
void indexDocs(const IndexWriterPtr &writer, const String &sourceDir)
{
    HashSet<String> dirList(HashSet<String>::newInstance());
    if (!FileUtils::listDirectory(sourceDir, false, dirList)) {
        return;
    }

    for (HashSet<String>::iterator dirFile = dirList.begin(); dirFile != dirList.end(); ++dirFile) {
        String docFile(FileUtils::joinPath(sourceDir, *dirFile));
        if (FileUtils::isDirectory(docFile) && StringUtils::toUTF8(*dirFile).at(0) != '.') {
            QString fullPath = StringUtils::toUTF8(docFile).c_str();
            QStringList pathList = fullPath.split('/');
            if (pathList.isEmpty() || pathList.size() < 2) {
                continue;
            }
            QString t_prefix = pathList.at(1);

            /*建立全文搜索索引排除以下目录 /boot /dev /proc /sys /root /run /lib  /usr*/
            if (t_prefix == "boot" || t_prefix == "dev" || t_prefix == "proc" || t_prefix == "sys"
                    || t_prefix == "root" || t_prefix == "run" || t_prefix == "lib" || t_prefix == "usr") {
                continue;
            } else {
                indexDocs(writer, docFile);
            }
        } else {
            if (StringUtils::toUTF8(*dirFile).at(0) != '.') {
                std::string input = wstring2utf8string(*dirFile);
                size_t last = input.find_last_of("/");
                std::string name = input.substr(last + 1);
                std::string dir = input.substr(0, last);
                std::string ext = input.substr(input.find_last_of(".") + 1);

                if (ext == "rtf" || ext == "odt" || ext == "ods" || ext == "odp" || ext == "odg"
                        || ext == "docx" || ext == "xlsx" || ext == "pptx" || ext == "ppsx"
                        || ext == "xls" || ext == "xlsb" || ext == "doc" || ext == "dot"
                        || ext == "wps" || ext == "ppt" || ext == "pps" || ext == "txt"
                        || ext == "htm" || ext == "html" || ext == "pdf") {
                    std::wcout << L"Adding [" << ++docNumber << L"]: " << docFile << L"\n";
                    try {
                        size_t last = input.find_last_of("/");
                        std::string name = input.substr(last + 1);
                        std::string dir = input.substr(0, last);
                        std::string ext = input.substr(input.find_last_of(".") + 1);
                        writer->addDocument(fileDocument(docFile));
                    } catch (FileNotFoundException &) {
                    }
                }
            }
        }
    }
}


//search
class OneNormsReader : public FilterIndexReader
{
public:
    OneNormsReader(const IndexReaderPtr &in, const String &field) : FilterIndexReader(in)
    {
        this->field = field;
    }

    virtual ~OneNormsReader()
    {
    }

protected:
    String field;

public:
    virtual ByteArray norms(const String &field)
    {
        return in->norms(this->field);
    }
};

static void doPagingSearch(const SearcherPtr &searcher, const QueryPtr &query, int32_t hitsPerPage, bool raw, bool interactive)
{
    // Collect enough docs to show 5 pages
    TopScoreDocCollectorPtr collector = TopScoreDocCollector::create(5 * hitsPerPage, false);
    searcher->search(query, collector);
    Collection<ScoreDocPtr> hits = collector->topDocs()->scoreDocs;

    int32_t numTotalHits = collector->getTotalHits();
    std::wcout << numTotalHits << L" total matching documents\n";

    int32_t start = 0;
    int32_t end = std::min(numTotalHits, hitsPerPage);

    while (true) {
        if (end > hits.size()) {
            std::wcout << L"Only results 1 - " << hits.size() << L" of " << numTotalHits << L" total matching documents collected.\n";
            std::wcout << L"Collect more (y/n) ?";
            String line;
            std::wcin >> line;
            boost::trim(line);

            if (line.empty() || boost::starts_with(line, L"n")) {
                break;
            }

            collector = TopScoreDocCollector::create(numTotalHits, false);
            searcher->search(query, collector);
            hits = collector->topDocs()->scoreDocs;
        }

        end = std::min(hits.size(), start + hitsPerPage);

        for (int32_t i = start; i < end; ++i) {
            if (raw) { // output raw format
                std::wcout << L"doc=" << hits[i]->doc << L" score=" << hits[i]->score << L"\n";
                continue;
            }

            DocumentPtr doc = searcher->doc(hits[i]->doc);
            String path = doc->get(L"path");
            if (!path.empty()) {
                std::wcout << StringUtils::toString(i + 1) + L". " << path << L"\n";
                searchResult->append(StringUtils::toUTF8(path).c_str());
                String title = doc->get(L"title");
                if (!title.empty()) {
                    std::wcout << L"   Title: " << doc->get(L"title") << L"\n";
                }
            } else {
                std::wcout << StringUtils::toString(i + 1) + L". No path for this document\n";
            }
        }

        if (!interactive) {
            break;
        }

        if (numTotalHits >= end) {
            break;
            end = std::min(numTotalHits, start + hitsPerPage);
        }
    }
}

class StreamingHitCollector : public Collector
{
public:
    StreamingHitCollector()
    {
        docBase = 0;
    }

    virtual ~StreamingHitCollector()
    {
    }

protected:
    ScorerPtr scorer;
    int32_t docBase;

public:
    /// simply print docId and score of every matching document
    virtual void collect(int32_t doc)
    {
        std::wcout << L"doc=" << (doc + docBase) << L" score=" << scorer->score();
    }

    virtual bool acceptsDocsOutOfOrder()
    {
        return true;
    }

    virtual void setNextReader(const IndexReaderPtr &reader, int32_t docBase)
    {
        this->docBase = docBase;
    }

    virtual void setScorer(const ScorerPtr &scorer)
    {
        this->scorer = scorer;
    }
};

/// This method uses a custom HitCollector implementation which simply prints out the docId and score of
/// every matching document.
///
/// This simulates the streaming search use case, where all hits are supposed to be processed, regardless
/// of their relevance.
static void doStreamingSearch(const SearcherPtr &searcher, const QueryPtr &query)
{
    searcher->search(query, newLucene<StreamingHitCollector>());
}
int search_keyworld(std::string keyword)
{
    try {
        auto indexPath = QStandardPaths::standardLocations(QStandardPaths::ConfigLocation).first();
        indexPath = indexPath
                    + "/" + QApplication::organizationName()
                    + "/" + QApplication::applicationName()
                    + "/" + "index";

        Lucene::String indexDir(indexPath.toStdWString());
        String index = indexDir;
        String field = L"contents";
        String queries;
        int32_t repeat = 0;
        bool raw = false;
        String normsField;
        bool paging = true;
        int32_t hitsPerPage = SEARCH_RESULT_NUM;
        // only searching, so read-only=true
        IndexReaderPtr reader = IndexReader::open(FSDirectory::open(index), true);

        if (!normsField.empty()) {
            reader = newLucene<OneNormsReader>(reader, normsField);
        }

        SearcherPtr searcher = newLucene<IndexSearcher>(reader);
        AnalyzerPtr analyzer = newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT);
        QueryParserPtr parser = newLucene<QueryParser>(LuceneVersion::LUCENE_CURRENT, field, analyzer);

        ReaderPtr in;
        if (!queries.empty()) {
            in = newLucene<FileReader>(queries);
        }

        QueryPtr query = parser->parse(StringUtils::toUnicode(keyword));
        std::wcout << L"Searching for: " << query->toString(field) << L"\n";

        if (repeat > 0) { // repeat and time as benchmark
            int64_t start = MiscUtils::currentTimeMillis();
            for (int32_t i = 0; i < repeat; ++i) {
                searcher->search(query, FilterPtr(), 100);
            }
            std::wcout << L"Time: " << (MiscUtils::currentTimeMillis() - start) << L"ms\n";
        }

        if (paging) {
            doPagingSearch(searcher, query, hitsPerPage, raw, queries.empty());
        } else {
            doStreamingSearch(searcher, query);
        }
        reader->close();
    } catch (LuceneException &e) {
        std::wcout << L"Exception: " << e.getError() << L"\n";
        return 1;
    }

    return 0;
}
int fulltextIndex(QString sourcefile)
{
    std::locale::global(std::locale(""));
    std::setlocale(LC_CTYPE, "");    // MinGW gcc.
    std::wcout.imbue(std::locale(""));

    auto indexPath = QStandardPaths::standardLocations(QStandardPaths::ConfigLocation).first();
    indexPath = indexPath
                + "/" + QApplication::organizationName()
                + "/" + QApplication::applicationName()
                + "/" + "index";

    Lucene::String indexDir(indexPath.toStdWString());

    Lucene::String sourceDir(StringUtils::toUnicode(sourcefile.toStdString()));
    if (!FileUtils::isDirectory(sourceDir)) {
        std::wcout << L"Source directory doesn't exist: " << sourceDir << L"\n";
        return 1;
    }

    if (!FileUtils::isDirectory(indexDir)) {
        if (!FileUtils::createDirectory(indexDir)) {
            std::wcout << L"Unable to create directory: " << indexDir << L"\n";
            return 1;
        }
    }

    uint64_t beginIndex = MiscUtils::currentTimeMillis();

    try {
        IndexWriterPtr writer = newLucene<IndexWriter>(FSDirectory::open(indexDir), newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), true, IndexWriter::MaxFieldLengthLIMITED);
        std::wcout << L"Indexing to directory: " << indexDir << L"...\n";
        writer->deleteAll();
        indexDocs(writer, sourceDir);

        uint64_t endIndex = MiscUtils::currentTimeMillis();
        uint64_t indexDuration = endIndex - beginIndex;
//        std::wcout << L"Index time: " << indexDuration << L" milliseconds\n";
//        std::wcout << L"Optimizing...\n";

        writer->optimize();

        uint64_t optimizeDuration = MiscUtils::currentTimeMillis() - endIndex;
//        std::wcout << L"Optimize time: " << optimizeDuration << L" milliseconds\n";

        writer->close();

        std::wcout << L"\n\nIndex time: " << indexDuration + optimizeDuration << L" milliseconds\n\n";
    } catch (LuceneException &e) {
        std::wcout << L"Exception: " << e.getError() << L"\n";
        return 1;
    }
}


DFMFullTextSearchManager::DFMFullTextSearchManager(QObject *parent)
    : QObject (parent)
{
    status = false;
}

DFMFullTextSearchManager *DFMFullTextSearchManager::getInstance()
{
    static DFMFullTextSearchManager instance;
    return &instance;
}
int DFMFullTextSearchManager::fulltextIndex(QString sourcefile)
{
    if (status == true) {
        return 0;
    } else {
        status = true;
        QtConcurrent::run([ = ] {
            fulltextIndexSetUp(sourcefile);
            status = false;
        });
        return 1;
    }
}

void DFMFullTextSearchManager::clearSearchResult()
{
    searchResult->clear();
}

int DFMFullTextSearchManager::fulltextIndexSetUp(QString sourcefile)
{
    std::locale::global(std::locale(""));
    std::setlocale(LC_CTYPE, "");    // MinGW gcc.
    std::wcout.imbue(std::locale(""));

    auto indexPath = QStandardPaths::standardLocations(QStandardPaths::ConfigLocation).first();
    indexPath = indexPath
                + "/" + QApplication::organizationName()
                + "/" + QApplication::applicationName()
                + "/" + "index";

    Lucene::String indexDir(indexPath.toStdWString());

    Lucene::String sourceDir(StringUtils::toUnicode(sourcefile.toStdString()));
    if (!FileUtils::isDirectory(sourceDir)) {
        std::wcout << L"Source directory doesn't exist: " << sourceDir << L"\n";
        return 1;
    }

    if (!FileUtils::isDirectory(indexDir)) {
        if (!FileUtils::createDirectory(indexDir)) {
            std::wcout << L"Unable to create directory: " << indexDir << L"\n";
            return 1;
        }
    }

    uint64_t beginIndex = MiscUtils::currentTimeMillis();

    try {
        IndexWriterPtr writer = newLucene<IndexWriter>(FSDirectory::open(indexDir), newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT), true, IndexWriter::MaxFieldLengthLIMITED);
        std::wcout << L"Indexing to directory: " << indexDir << L"...\n";
        writer->deleteAll();
        indexDocs(writer, sourceDir);

        uint64_t endIndex = MiscUtils::currentTimeMillis();
        uint64_t indexDuration = endIndex - beginIndex;
        writer->optimize();

        uint64_t optimizeDuration = MiscUtils::currentTimeMillis() - endIndex;

        writer->close();

        std::wcout << L"\n\nIndex time: " << indexDuration + optimizeDuration << L" milliseconds\n\n";
        qDebug() << "\n\nIndex time: " << indexDuration + optimizeDuration << " milliseconds\n\n";
    } catch (LuceneException &e) {
        std::wcout << L"Exception: " << e.getError() << L"\n";
        return 1;
    }
    return 0;
}

/// Index all text files under a directory.
QStringList DFMFullTextSearchManager::fulltextSearch(QString keyword)
{
    search_keyworld(keyword.toStdString());

    return *searchResult;
}
DFM_END_NAMESPACE
