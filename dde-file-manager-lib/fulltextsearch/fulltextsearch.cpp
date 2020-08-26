/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <boost/algorithm/string.hpp>
#include <codecvt>
#include <dirent.h>
#include <fnmatch.h>

//lucene++ header
#include <FileUtils.h>
#include <FilterIndexReader.h>
#include <FuzzyQuery.h>

//doctotext header
#include "misc.h"
#include "plain_text_extractor.h"

#include <QtConcurrentRun>
#include <QStringList>
#include <QStandardPaths>
#include <QApplication>
#include <QDir>
#include <QFileInfoList>

#include "fulltextsearch.h"
#include "dfmapplication.h"

DFM_BEGIN_NAMESPACE
#define SEARCH_RESULT_NUM   100000

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

DFMFullTextSearchManager::DFMFullTextSearchManager(QObject *parent)
    : QThread (parent)
{
    status = false;
    indexStorePath = QStandardPaths::standardLocations(QStandardPaths::ConfigLocation).first()
                     + "/" + QApplication::organizationName()
                     + "/" + QApplication::applicationName()
                     + "/" + "index";
}

DFMFullTextSearchManager *DFMFullTextSearchManager::getInstance()
{
    static DFMFullTextSearchManager instance;
    return &instance;
}

QString DFMFullTextSearchManager::getFileContents(const QString &filePath)
{
    std::string text = "";
    FormattingStyle options;
    options.table_style = TABLE_STYLE_TABLE_LOOK;
    options.list_style.setPrefix(" * ");
    options.url_style = URL_STYLE_UNDERSCORED;
    XmlParseMode mode = PARSE_XML;

    PlainTextExtractor::ParserType parser_type = PlainTextExtractor::PARSER_AUTO;
    QFileInfo fileInfo(filePath);
    QString ext = fileInfo.suffix();
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
    else if (ext == "ppt" || ext == "pps" || ext == "dps")
        parser_type = PlainTextExtractor::PARSER_PPT;
    else if (ext == "pdf")
        parser_type = PlainTextExtractor::PARSER_PDF;
    else if (ext == "txt" || ext == "text")
        parser_type = PlainTextExtractor::PARSER_TXT;
    else {
        qDebug() << "Unsupported file extension: " << ext;
        return "";
    }

    //创建文件解析器
    PlainTextExtractor extractor(parser_type);

    //设置 xml 格式解析模式
    if (mode != PARSE_XML) {
        extractor.setXmlParseMode(mode);
    }

    //
    extractor.setFormattingStyle(options);

    //解析文件内容,
    /*
     * 原因是解析出来的内容和 tests目录下面 *.out 中的内容不相符，
     * 例如: 解析 tests/1.doc.out 最终结果的内容就是现在下面 printf 所打印输出的内容格式，
     * 从可以 tests/Makefile 中看到，在测试的过程中是通过 diff 命令比较，也就是通过比较 1.doc
     * 的解析结果内容和 1.doc.out 内容的是否相同来判断程序是否正常运行的
     */
    if (!extractor.processFile(filePath.toStdString(), text)) {
        qDebug() << "Error processing file " << filePath;
        return "";
    }
    return text.c_str();
}

//先将字段加入到文档,在将文档加入到IndexWriter中
DocumentPtr DFMFullTextSearchManager::getFileDocument(const QString &filename)
{
    DocumentPtr doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"path", filename.toStdWString(), Field::STORE_YES, Field::INDEX_ANALYZED));
    QFileInfo fileInfo(filename);
    QString modifyTime = fileInfo.lastModified().toString("yyyyMMddHHmmss");
    doc->add(newLucene<Field>(L"modified", modifyTime.toStdWString(), Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
    QString contents = getFileContents(filename);
    doc->add(newLucene<Field>(L"contents", contents.toStdWString(), Field::STORE_YES, Field::INDEX_ANALYZED));
    return doc;
}
void DFMFullTextSearchManager::indexDocs(const IndexWriterPtr &writer, const QString &sourceDir)
{
    isCreateIndex = true;
    QStringList files;
    readFileName(sourceDir.toStdString().c_str(), files);
    isCreateIndex = false;
    for (auto file : files) {
        qDebug() << "Adding [" << file << "]";
        try {
            writer->addDocument(getFileDocument(file));
        } catch (FileNotFoundException &ex) {
            qDebug() << "addDocument error: " << ex.getError().c_str();
        }
    }
}

void DFMFullTextSearchManager::doPagingSearch(const SearcherPtr &searcher, const QueryPtr &query, int32_t hitsPerPage, bool raw, bool interactive)
{
    // Collect enough docs to show 5 pages
    TopScoreDocCollectorPtr collector = TopScoreDocCollector::create(5 * hitsPerPage, false);
    searcher->search(query, collector);
    Collection<ScoreDocPtr> hits = collector->topDocs()->scoreDocs;

    int32_t numTotalHits = collector->getTotalHits();
    qDebug() << numTotalHits << " total matching documents";

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
            if (m_state == JobController::Stoped) {
                return;
            }

            if (raw) { // output raw format
                std::wcout << L"doc=" << hits[i]->doc << L" score=" << hits[i]->score << L"\n";
                continue;
            }

            DocumentPtr doc = searcher->doc(hits[i]->doc);
            String path = doc->get(L"path");
            if (!path.empty()) {
                qDebug() << QString::number(i + 1) << " " << QString::fromStdWString(path.c_str());
                searchResults.append(StringUtils::toUTF8(path).c_str());
            } else {
                qDebug() << QString::number(i + 1).append(". No path for this document");
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

bool DFMFullTextSearchManager::searchByKeyworld(const QString &keyword)
{
    try {
        int32_t hitsPerPage = SEARCH_RESULT_NUM;
        // only searching, so read-only=true
        IndexReaderPtr reader = IndexReader::open(FSDirectory::open(indexStorePath.toStdWString()), true);

        SearcherPtr searcher = newLucene<IndexSearcher>(reader);
        AnalyzerPtr analyzer = newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT);
        QueryParserPtr parser = newLucene<QueryParser>(LuceneVersion::LUCENE_CURRENT, L"contents", analyzer);
        parser->setAllowLeadingWildcard(true);//设定第一个* 可以匹配

        QueryPtr query = nullptr;
        // 判断是否包含中文
        if (keyword.contains(QRegExp("[\\x4e00-\\x9fa5]+"))) {
            query = parser->parse(keyword.toStdWString());
        } else {
            QString word = "*" + keyword + "*";
            query = parser->parse(word.toStdWString());
        }
        qDebug() << "Searching for: " << keyword;

        doPagingSearch(searcher, query, hitsPerPage, false, true);
        reader->close();
    } catch (LuceneException &e) {
        qDebug() << "Exception: " << e.getError().c_str();
        return false;
    }

    return true;
}

int DFMFullTextSearchManager::fulltextIndex(const QString &sourceDir)
{
    if (status == true) {
        return 0;
    } else {
        status = true;
        QtConcurrent::run([ = ] {
            createFileIndex(sourceDir);
            status = false;
        });
        return 1;
    }
}

void DFMFullTextSearchManager::readFileName(const char *filePath, QStringList &result)
{
    if (!isCreateIndex && m_state == JobController::Stoped) {
        return;
    }

    QRegExp reg("^/(boot|dev|proc|sys|run|lib|usr).*$");
    if (reg.exactMatch(QString(filePath)) && !QString(filePath).startsWith("/run/user")) {
        return;
    }

    std::string path(filePath);
    int pathLevel = 0;
    std::for_each(std::begin(path), std::end(path), [&](char ch) {
        if (ch == '/') {
            pathLevel++;
        }
    });

    if (pathLevel > 10) {
        return;
    }

    DIR *dir = NULL;
    if (!(dir = opendir (filePath))) {
        //trace ("can't open: %s\n", dname);
        return;
    }
    struct dirent *dent = NULL;
    int len = strlen (filePath);
    if (len >= FILENAME_MAX - 1) {
        //trace ("filename too long: %s\n", dname);
        return;
    }

    char fn[FILENAME_MAX] = "";
    strcpy (fn, filePath);
    if (strcmp (filePath, "/")) {
        // TODO: use a more performant fix to handle root directory
        fn[len++] = '/';
    }
    while ((dent = readdir (dir))) {
        if (!dent->d_name[0] == '.') {
            // file is dotfile, skip
            continue;
        }
        if (!strcmp (dent->d_name, ".") || !strcmp (dent->d_name, "..") || !strcmp (dent->d_name, ".avfs")) {
            continue;
        }

        struct stat st;
        strncpy (fn + len, dent->d_name, FILENAME_MAX - len);
        if (lstat (fn, &st) == -1) {
            //warn("Can't stat %s", fn);
            continue;
        }
        const bool is_dir = S_ISDIR (st.st_mode);
        if (is_dir) {
            readFileName(fn, result);
        } else {
            QFileInfo fileInfo(fn);
            QString suffix = fileInfo.suffix();
            QRegExp reg("(rtf)|(odt)|(ods)|(odp)|(odg)|(docx)|(xlsx)|(pptx)|(ppsx)|"
                        "(xls)|(xlsb)|(doc)|(dot)|(wps)|(ppt)|(pps)|(txt)|(htm)|(html)|(pdf)|(dps)");
            if (reg.exactMatch(suffix)) {
                result.append(fn);
            }
        }
    }
    if (dir) {
        closedir (dir);
    }
}

void DFMFullTextSearchManager::updateIndex(const QString &filePath)
{
    qDebug() << "Update index";
    QStringList files;
    readFileName(filePath.toStdString().c_str(), files);
    for (QString file : files) {
        if (m_state == JobController::Stoped) {
            qDebug() << "full text search stop!";
            return;
        }

        try {
            IndexReaderPtr reader = IndexReader::open(FSDirectory::open(indexStorePath.toStdWString()), true);
            SearcherPtr searcher = newLucene<IndexSearcher>(reader);
            AnalyzerPtr analyzer = newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT);
            QueryParserPtr parser = newLucene<QueryParser>(LuceneVersion::LUCENE_CURRENT, L"path", analyzer);
            QueryPtr query = parser->parse(file.toStdWString());

            // 文件路径为唯一值，所以搜索一个结果就行了
            TopScoreDocCollectorPtr collector = TopScoreDocCollector::create(1, false);
            searcher->search(query, collector);
            Collection<ScoreDocPtr> hits = collector->topDocs()->scoreDocs;
            int32_t numTotalHits = collector->getTotalHits();
            if (numTotalHits == 0) {
                try {
                    IndexWriterPtr writer = newLucene<IndexWriter>(FSDirectory::open(indexStorePath.toStdWString()),
                                                                   newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT),
                                                                   IndexWriter::MaxFieldLengthLIMITED);
                    writer->addDocument(getFileDocument(file));
                    qDebug() << "Add file: [" << file << "]";
                    writer->close();
                } catch (LuceneException &ex) {
                    qDebug() << ex.getError().c_str();
                }

            } else {
                DocumentPtr doc = searcher->doc(hits[0]->doc);
                QFileInfo info(file);
                QString modifyTime = info.lastModified().toString("yyyyMMddHHmmss");
                String storeTime = doc->get(L"modified");
                if (modifyTime.toStdWString() == storeTime) {
                    continue;
                } else {
                    try {
                        IndexWriterPtr writer = newLucene<IndexWriter>(FSDirectory::open(indexStorePath.toStdWString()),
                                                                       newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT),
                                                                       IndexWriter::MaxFieldLengthLIMITED);
                        //定义一个更新条件
                        TermPtr query = newLucene<Term>(L"path", file.toStdWString());
                        //更新
                        writer->updateDocument(query, getFileDocument(file));
                        qDebug() << "Update file: [" << file << "]";
                        //关闭
                        writer->close();
                    } catch (LuceneException &ex) {
                        qDebug() << ex.getError().c_str();
                    }
                }
            }
            reader->close();
        } catch (LuceneException &ex) {
            String err = ex.getError();
            qDebug() << ex.getError().c_str() << " Type: " << ex.getType();
        }
    }
}

bool DFMFullTextSearchManager::createFileIndex(const QString &sourcePath)
{
    QDir sourceDir(sourcePath);
    if (!sourceDir.exists()) {
        qDebug() << "Source directory doesn't exist: " << sourcePath;
        return false;
    }

    QDir indexDir(indexStorePath);
    if (!indexDir.exists()) {
        if (!indexDir.mkpath(indexStorePath)) {
            qDebug() << "Unable to create directory: " << indexStorePath;
            return false;
        }
    }

    uint64_t beginIndex = MiscUtils::currentTimeMillis();

    try {
        IndexWriterPtr writer = newLucene<IndexWriter>(FSDirectory::open(indexStorePath.toStdWString()),
                                                       newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT),
                                                       true,
                                                       IndexWriter::MaxFieldLengthLIMITED);
        qDebug() << "Indexing to directory: " << indexStorePath;
        writer->deleteAll();
        indexDocs(writer, sourcePath);

        uint64_t endIndex = MiscUtils::currentTimeMillis();
        uint64_t indexDuration = endIndex - beginIndex;
        writer->optimize();

        uint64_t optimizeDuration = MiscUtils::currentTimeMillis() - endIndex;
        writer->close();

        qDebug() << "\n\nIndex time: " << indexDuration + optimizeDuration << " milliseconds\n\n";
    } catch (LuceneException &e) {
        qDebug() << "Exception: " << e.getError().c_str();
        return false;
    }
    return true;
}

QStringList DFMFullTextSearchManager::fullTextSearch(const QString &keyword)
{
    searchResults.clear();
    if (searchByKeyworld(keyword))
        return searchResults;

    return QStringList();
}
DFM_END_NAMESPACE
