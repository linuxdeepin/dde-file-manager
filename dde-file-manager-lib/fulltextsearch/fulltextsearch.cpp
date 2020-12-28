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
#include <QueryWrapperFilter.h>

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
#include "chineseanalyzer.h"

DFM_BEGIN_NAMESPACE
#define SEARCH_RESULT_NUM   100000

DFMFullTextSearchManager::DFMFullTextSearchManager(QObject *parent)
    : QObject(parent)
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
    QFileInfo info(filePath);
    QString ext = info.suffix();
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
    return QString(text.c_str());
}

//先将字段加入到文档,在将文档加入到IndexWriter中
DocumentPtr DFMFullTextSearchManager::getFileDocument(const QString &filename)
{
    DocumentPtr doc = newLucene<Document>();
    doc->add(newLucene<Field>(L"path", filename.toStdWString(), Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
    QFileInfo info(filename);
    QString modifyTime = info.lastModified().toString("yyyyMMddHHmmss");
    doc->add(newLucene<Field>(L"modified", modifyTime.toStdWString(), Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
    QString contents = getFileContents(filename);
    doc->add(newLucene<Field>(L"contents", contents.toStdWString(), Field::STORE_YES, Field::INDEX_ANALYZED));
    return doc;
}
void DFMFullTextSearchManager::indexDocs(const IndexWriterPtr &writer, const QString &sourceDir)
{
    isCreateIndex = true;
    QStringList files;
    traverseFloder(sourceDir.toStdString().c_str(), files);
    isCreateIndex = false;
    for (auto file : files) {
        qDebug() << "Adding [" << file << "]";
        try {
            writer->addDocument(getFileDocument(file));
        } catch (FileNotFoundException &ex) {
            qDebug() << "addDocument error: " << QString::fromStdWString(ex.getError());
        }
    }
}

void DFMFullTextSearchManager::doSearch(const SearcherPtr &searcher, const QueryPtr &query, const QString &searchPath)
{
    // 创建过滤器，过滤掉非搜索目录的结果
    String filterPath = searchPath.endsWith("/") ? (searchPath + "*").toStdWString() : (searchPath + "/*").toStdWString();
    FilterPtr filter = newLucene<QueryWrapperFilter>(newLucene<WildcardQuery>(newLucene<Term>(L"path", filterPath)));
    TopDocsPtr topDocs = searcher->search(query, filter, SEARCH_RESULT_NUM);
    Collection<ScoreDocPtr> scoreDocs = topDocs->scoreDocs;
    int resNum = 0;
    for (auto scoreDoc : scoreDocs) {
        DocumentPtr doc = searcher->doc(scoreDoc->doc);
        String path = doc->get(L"path");

        if (!path.empty()) {
            /*fix bug 45096 对搜索结果里面的修改时间进行判断，如果时间不一样则表示文件已经更改，搜索结果不出现该文件*/
            QFileInfo info(QString::fromStdWString(path));
            QString modifyTime = info.lastModified().toString("yyyyMMddHHmmss");
            String storeTime = doc->get(L"modified");
            if (modifyTime.toStdWString() != storeTime) {
                continue;
            } else {
                searchResults.append(StringUtils::toUTF8(path).c_str());
                qDebug() << ++resNum << " " << QString::fromStdWString(path.c_str());
            }
        }
    }
}

bool DFMFullTextSearchManager::searchByKeyworld(const QString &keyword, const QString &searchPath)
{
    try {
        // only searching, so read-only=true
        IndexReaderPtr reader = IndexReader::open(FSDirectory::open(indexStorePath.toStdWString()), true);

        SearcherPtr searcher = newLucene<IndexSearcher>(reader);
        AnalyzerPtr analyzer = newLucene<ChineseAnalyzer>();
        QueryParserPtr parser = newLucene<QueryParser>(LuceneVersion::LUCENE_CURRENT, L"contents", analyzer);
        parser->setAllowLeadingWildcard(true);//设定第一个* 可以匹配

        QString newKeyWorld = dealKeyWorld(keyword);
        QueryPtr query = parser->parse(newKeyWorld.toStdWString());
        qDebug() << "Searching for: " << keyword;

        doSearch(searcher, query, searchPath);
        reader->close();
    } catch (LuceneException &e) {
        qDebug() << "Exception: " << QString::fromStdWString(e.getError());
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

void DFMFullTextSearchManager::traverseFloder(const char *filePath, QStringList &result)
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
    /*目录级数限定*/
    if (pathLevel > 20) {
        return;
    }

    DIR *dir = nullptr;
    if (!(dir = opendir(filePath))) {
        //trace ("can't open: %s\n", dname);
        return;
    }
    struct dirent *dent = nullptr;
    size_t len = strlen(filePath);
    if (len >= FILENAME_MAX - 1) {
        //trace ("filename too long: %s\n", dname);
        return;
    }

    char fn[FILENAME_MAX] = "";
    strcpy(fn, filePath);
    if (strcmp(filePath, "/")) {
        // TODO: use a more performant fix to handle root directory
        fn[len++] = '/';
    }
    while ((dent = readdir(dir))) {
        if ((QString(dent->d_name).at(0) == '.') && !QString(dent->d_name).startsWith(".local")) {
            continue;
        }
        if (!strcmp(dent->d_name, ".") || !strcmp(dent->d_name, "..") || !strcmp(dent->d_name, ".avfs")) {
            continue;
        }

        struct stat st;
        strncpy(fn + len, dent->d_name, FILENAME_MAX - len);
        if (lstat(fn, &st) == -1) {
            //warn("Can't stat %s", fn);
            continue;
        }
        const bool is_dir = S_ISDIR(st.st_mode);
        if (is_dir) {
            traverseFloder(fn, result);
        } else {
            QFileInfo info(fn);
            QString suffix = info.suffix();
            QRegExp regExp("(rtf)|(odt)|(ods)|(odp)|(odg)|(docx)|(xlsx)|(pptx)|(ppsx)|"
                           "(xls)|(xlsb)|(doc)|(dot)|(wps)|(ppt)|(pps)|(txt)|(htm)|(html)|(pdf)|(dps)");
            if (regExp.exactMatch(suffix)) {
                result.append(fn);
            }
        }
    }
    if (dir) {
        closedir(dir);
    }
}

QString DFMFullTextSearchManager::dealKeyWorld(const QString &keyWorld)
{
    QRegExp cnReg("^[\u4e00-\u9fa5]"), enReg("^[A-Za-z]+$"), numReg("^[0-9]$");
    CharType oldType = CN, currType = CN;
    QString newStr;
    for (auto c : keyWorld) {
        if (cnReg.exactMatch(c)) {
            currType = CN;
        } else if (enReg.exactMatch(c)) {
            currType = EN;
        } else if (numReg.exactMatch(c)) {
            currType = DIGIT;
        } else if (c == ' ') {
            currType = SPACE;
        } else {
            continue;
        }

        newStr += c;
        // 如果当前字符或者上一个字符是空格，则不需要再加空格
        if (oldType == SPACE || currType == SPACE) {
            oldType = currType;
            continue;
        }

        if (oldType != currType) {
            oldType = currType;
            newStr.insert(newStr.length() - 1, " ");
        }
    }

    return newStr.trimmed();
}

bool DFMFullTextSearchManager::updateIndex(const QString &filePath)
{
    bool res = false;
    qDebug() << "Update index";
    QStringList files;
    traverseFloder(filePath.toStdString().c_str(), files);
    for (QString file : files) {
        if (m_state == JobController::Stoped) {
            qDebug() << "full text search stop!";
            return false;
        }

        try {
            IndexReaderPtr reader = IndexReader::open(FSDirectory::open(indexStorePath.toStdWString()), true);
            SearcherPtr searcher = newLucene<IndexSearcher>(reader);
            TermQueryPtr query = newLucene<TermQuery>(newLucene<Term>(L"path", file.toStdWString()));

            // 文件路径为唯一值，所以搜索一个结果就行了
            TopDocsPtr topDocs = searcher->search(query, 1);
            int32_t numTotalHits = topDocs->totalHits;
            if (numTotalHits == 0) {
                try {
                    IndexWriterPtr writer = newLucene<IndexWriter>(FSDirectory::open(indexStorePath.toStdWString()),
                                                                   newLucene<ChineseAnalyzer>(),
                                                                   IndexWriter::MaxFieldLengthLIMITED);
                    writer->addDocument(getFileDocument(file));
                    qDebug() << "Add file: [" << file << "]";
                    writer->close();
                    res = true;
                } catch (LuceneException &ex) {
                    qDebug() << "Add file error:" << QString::fromStdWString(ex.getError());
                }

            } else {
                DocumentPtr doc = searcher->doc(topDocs->scoreDocs[0]->doc);
                QFileInfo info(file);
                QString modifyTime = info.lastModified().toString("yyyyMMddHHmmss");
                String storeTime = doc->get(L"modified");
                if (modifyTime.toStdWString() == storeTime) {
                    continue;
                } else {
                    try {
                        IndexWriterPtr writer = newLucene<IndexWriter>(FSDirectory::open(indexStorePath.toStdWString()),
                                                                       newLucene<ChineseAnalyzer>(),
                                                                       IndexWriter::MaxFieldLengthLIMITED);
                        //定义一个更新条件
                        TermPtr term = newLucene<Term>(L"path", file.toStdWString());
                        //更新
                        writer->updateDocument(term, getFileDocument(file));
                        qDebug() << "Update file: [" << file << "]";
                        //关闭
                        writer->close();
                        res = true;
                    } catch (LuceneException &ex) {
                        qDebug() << "Update file error:" << QString::fromStdWString(ex.getError());
                    }
                }
            }
            reader->close();
        } catch (LuceneException &ex) {
            qDebug() << QString::fromStdWString(ex.getError());
        }
    }

    return res;
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
                                                       newLucene<ChineseAnalyzer>(),
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
        qDebug() << "Exception: " << QString::fromStdWString(e.getError());
        return false;
    }
    return true;
}

QStringList DFMFullTextSearchManager::fullTextSearch(const QString &keyword, const QString &searchPath)
{
    searchResults.clear();
    if (searchByKeyworld(keyword, searchPath))
        return searchResults;

    return QStringList();
}
DFM_END_NAMESPACE
