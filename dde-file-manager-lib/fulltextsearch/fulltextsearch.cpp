/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <boost/algorithm/string.hpp>
#include <codecvt>

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
    clock_t start, end;
    start = clock();           /*记录起始时间*/

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
    qDebug() << "Conversion complete: " << filePath;

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

    end = clock();           /*记录结束时间*/
    double seconds  = (double)(end - start) / CLOCKS_PER_SEC;
    qDebug() << "Use time is: " << seconds;

    return text.c_str();
}

//先将字段加入到文档,在将文档加入到IndexWriter中
DocumentPtr DFMFullTextSearchManager::getFileDocument(const QString &filename)
{
    DocumentPtr doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"path", filename.toStdWString(), Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
    QString contents = getFileContents(filename);
    doc->add(newLucene<Field>(L"contents", contents.toStdWString(), Field::STORE_YES, Field::INDEX_ANALYZED));
    return doc;
}
void DFMFullTextSearchManager::indexDocs(const IndexWriterPtr &writer, const QString &sourceDir)
{
    QDir searchDir(sourceDir);
    if (!searchDir.exists()) {
        return;
    }

    // 限制一下目录级数
    QStringList dirLevel = sourceDir.split('/');
    if (dirLevel.count() > 10) {
        return;
    }

    //遍历目录文件
    QFileInfoList fileInfoList = searchDir.entryInfoList(QDir::AllEntries | QDir::Hidden);
    for (auto fileInfo : fileInfoList) {
        QString filePath = fileInfo.filePath();
        QString fileName = fileInfo.fileName();
        if (fileInfo.isDir()) {
            /*建立全文搜索索引排除以下目录 /boot /dev /proc /sys /root /run /lib  /usr*/
            QRegExp reg("^/(boot|dev|proc|sys|root|lib|usr).*$");
            if (reg.exactMatch(filePath) || fileName == "." || fileName == ".." || fileName == ".avfs") {
                continue;
            } else {
                indexDocs(writer, filePath);
            }
        } else {
            QString suffix = fileInfo.suffix();
            QRegExp reg("(rtf)|(odt)|(ods)|(odp)|(odg)|(docx)|(xlsx)|(pptx)|(ppsx)|"
                        "(xls)|(xlsb)|(doc)|(dot)|(wps)|(ppt)|(pps)|(txt)|(htm)|(html)|(pdf)|(dps)");
            if (reg.exactMatch(suffix)) {
                qDebug() << "Adding [" << filePath << "]";
                try {
                    if (fileInfo.exists())
                        writer->addDocument(getFileDocument(filePath));
                } catch (FileNotFoundException &ex) {
                    qDebug() << "addDocument error: " << ex.getError().c_str();
                }
            }
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

void DFMFullTextSearchManager::run()
{
    while (!indexQueue.isEmpty()) {
        mutex.lock();
        auto info = indexQueue.dequeue();
        mutex.unlock();

        updateIndex(info.first, info.second);
    }
    m_state = Stoped;
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

void DFMFullTextSearchManager::start()
{
    if (m_state == Started)
        return;

    m_state = Started;
    QThread::start();
}

void DFMFullTextSearchManager::addIndexQueue(const QString &filePath, DFMFullTextSearchManager::Type type)
{
    mutex.lock();
    indexQueue.enqueue(qMakePair(filePath, type));
    mutex.unlock();

    start();
}

void DFMFullTextSearchManager::updateIndex(const QString &filePath, DFMFullTextSearchManager::Type type)
{
    if (filePath.contains(".avfs"))
        return;

    QRegExp reg("^/(boot|dev|proc|sys|root|run|lib|usr).*$");
    if (reg.exactMatch(filePath) && !filePath.startsWith("/run/user")) {
        return;
    }

    switch (type) {
    case Add: {
        try {
            QFileInfo fileInfo(filePath);
            if (fileInfo.isFile()) {
                QString suffix = fileInfo.suffix();
                reg.setPattern("(rtf)|(odt)|(ods)|(odp)|(odg)|(docx)|(xlsx)|(pptx)|(ppsx)|"
                               "(xls)|(xlsb)|(doc)|(dot)|(wps)|(ppt)|(pps)|(txt)|(htm)|(html)|(pdf)|(dps)");
                if (reg.exactMatch(suffix)) {
                    IndexWriterPtr writer = newLucene<IndexWriter>(FSDirectory::open(indexStorePath.toStdWString()),
                                                                   newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT),
                                                                   IndexWriter::MaxFieldLengthLIMITED);
                    if (fileInfo.exists()) {
                        writer->addDocument(getFileDocument(filePath));
                        // 为防止出现重复索引，需要更新一次
                        TermPtr query = newLucene<Term>(L"path", filePath.toStdWString());
                        //更新
                        writer->updateDocument(query, getFileDocument(filePath));
                        qDebug() << "Add [" << filePath << "]";
                        writer->close();
                    }
                }
            } else {
                QDir dir(filePath);
                QFileInfoList pathList = dir.entryInfoList(QDir::AllEntries | QDir::Hidden);
                for (QFileInfo path : pathList) {
                    QString fileName = path.fileName();
                    if (fileName == "." || fileName == "..") {
                        continue;
                    }
                    updateIndex(path.filePath(), Add);
                }
            }
        } catch (FileNotFoundException &ex) {
            qDebug() << "addDocument error: " << ex.getError().c_str();
        }
    }
    break;
    case Modify: {
        try {
            QFileInfo fileInfo(filePath);
            if (fileInfo.isFile()) {
                QString suffix = fileInfo.suffix();
                reg.setPattern("(rtf)|(odt)|(ods)|(odp)|(odg)|(docx)|(xlsx)|(pptx)|(ppsx)|"
                               "(xls)|(xlsb)|(doc)|(dot)|(wps)|(ppt)|(pps)|(txt)|(htm)|(html)|(pdf)|(dps)");
                if (reg.exactMatch(suffix)) {
                    IndexWriterPtr writer = newLucene<IndexWriter>(FSDirectory::open(indexStorePath.toStdWString()),
                                                                   newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT),
                                                                   IndexWriter::MaxFieldLengthLIMITED);
                    if (fileInfo.exists()) {
                        //定义一个更新条件
                        TermPtr query = newLucene<Term>(L"path", filePath.toStdWString());
                        //更新
                        writer->updateDocument(query, getFileDocument(filePath));
                        qDebug() << "Update [" << filePath << "]";
                        //关闭
                        writer->close();
                    }
                }
            }
        } catch (FileNotFoundException &ex) {
            qDebug() << "addDocument error: " << ex.getError().c_str();
        }
    }
    break;
    case Delete: {
        try {
            IndexWriterPtr writer = newLucene<IndexWriter>(FSDirectory::open(indexStorePath.toStdWString()),
                                                           newLucene<StandardAnalyzer>(LuceneVersion::LUCENE_CURRENT),
                                                           IndexWriter::MaxFieldLengthLIMITED);
            //定义一个删除条件，定义一个查询对象
            QueryPtr query = newLucene<TermQuery>(newLucene<Term>(L"path", filePath.toStdWString()));
            //删除
            writer->deleteDocuments(query);
            qDebug() << "Delete [" << filePath << "]";
            //关闭
            writer->close();
        } catch (FileNotFoundException &ex) {
            qDebug() << "deleteDocuments error: " << ex.getError().c_str();
        }
    }
    break;
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
