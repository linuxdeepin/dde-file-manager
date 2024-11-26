// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "taskhandler.h"

#include "progressnotifier.h"

#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/finallyutil.h>

#include <docparser.h>

#include <fulltext/chineseanalyzer.h>

#include <lucene++/LuceneHeaders.h>
#include <FileUtils.h>
#include <FilterIndexReader.h>
#include <FuzzyQuery.h>
#include <QueryWrapperFilter.h>

#include <QDir>
#include <QDirIterator>
#include <QDateTime>
#include <QRegularExpression>
#include <QStandardPaths>

#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

SERVICETEXTINDEX_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
using namespace Lucene;

namespace {

static constexpr char kFilterFolders[] { "^/(boot|dev|proc|sys|run|lib|usr).*$" };
static constexpr char kSupportFiles[] { "(rtf)|(odt)|(ods)|(odp)|(odg)|(docx)|(xlsx)|(pptx)|(ppsx)|(md)|"
                                        "(xls)|(xlsb)|(doc)|(dot)|(wps)|(ppt)|(pps)|(txt)|(pdf)|(dps)|"
                                        "(sh)|(html)|(htm)|(xml)|(xhtml)|(dhtml)|(shtm)|(shtml)|"
                                        "(json)|(css)|(yaml)|(ini)|(bat)|(js)|(sql)|(uof)|(ofd)" };

DocumentPtr createFileDocument(const QString &file)
{
    DocumentPtr doc = newLucene<Document>();

    // file path
    doc->add(newLucene<Field>(L"path", file.toStdWString(),
                              Field::STORE_YES, Field::INDEX_NOT_ANALYZED));

    // file last modified time
    auto info = InfoFactory::create<FileInfo>(QUrl::fromLocalFile(file),
                                              Global::CreateFileInfoType::kCreateFileInfoSync);
    const QDateTime &modifyTime { info->timeOf(TimeInfoType::kLastModified).toDateTime() };
    const QString &modifyEpoch { QString::number(modifyTime.toSecsSinceEpoch()) };
    doc->add(newLucene<Field>(L"modified", modifyEpoch.toStdWString(),
                              Field::STORE_YES, Field::INDEX_NOT_ANALYZED));

    // file contents
    QString contents = DocParser::convertFile(file.toStdString()).c_str();
    doc->add(newLucene<Field>(L"contents", contents.toStdWString(),
                              Field::STORE_YES, Field::INDEX_ANALYZED));

    return doc;
}

void processFile(const QString &path, const IndexWriterPtr &writer, qint64 *processedCount)
{
    try {
        auto info = InfoFactory::create<FileInfo>(QUrl::fromLocalFile(path),
                                                  Global::CreateFileInfoType::kCreateFileInfoSync);
        if (!info) return;

        QString suffix = info->nameOf(NameInfoType::kSuffix);
        static const QRegularExpression suffixRegex(kSupportFiles);
        if (suffixRegex.match(suffix).hasMatch()) {
            fmDebug() << "Adding [" << path << "]";
            writer->addDocument(createFileDocument(path));
            if (processedCount) {
                ++(*processedCount);
                emit ProgressNotifier::instance()->progressChanged(*processedCount);
            }
        }
    } catch (const std::exception &e) {
        fmWarning() << "Process file failed:" << path << e.what();
    }
}

void traverseDirectory(const QString &rootPath, const IndexWriterPtr &writer,
                       qint64 *processedCount, const std::atomic_bool &running)
{
    static const QRegularExpression filterRegex(kFilterFolders);
    QMap<QString, QString> bindPathTable = DeviceUtils::fstabBindInfo();

    QQueue<QString> dirQueue;
    dirQueue.enqueue(rootPath);

    while (!dirQueue.isEmpty() && running.load()) {
        QString currentPath = dirQueue.dequeue();

        if (bindPathTable.contains(currentPath) || 
            (filterRegex.match(currentPath).hasMatch() && !currentPath.startsWith("/run/user")))
            continue;

        if (currentPath.size() > FILENAME_MAX - 1 || currentPath.count('/') > 20)
            continue;

        DIR *dir = opendir(currentPath.toStdString().c_str());
        if (!dir) {
            fmWarning() << "Cannot open directory:" << currentPath;
            continue;
        }

        FinallyUtil dirCloser([dir]() { closedir(dir); });

        struct dirent *entry;
        while ((entry = readdir(dir)) && running.load()) {
            if (entry->d_name[0] == '.' && strncmp(entry->d_name, ".local", strlen(".local")))
                continue;

            if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
                continue;

            QString fullPath = currentPath + "/" + entry->d_name;
            struct stat st;
            if (lstat(fullPath.toStdString().c_str(), &st) == -1)
                continue;

            if (S_ISDIR(st.st_mode)) {
                dirQueue.enqueue(fullPath);
            } else {
                processFile(fullPath, writer, processedCount);
            }
        }
    }
}

bool checkNeedUpdate(const QString &file, const IndexReaderPtr &reader, bool *needAdd)
{
    try {
        SearcherPtr searcher = newLucene<IndexSearcher>(reader);
        TermQueryPtr query = newLucene<TermQuery>(newLucene<Term>(L"path", file.toStdWString()));

        // 文件路径为唯一值，所以搜索一个结果就行了
        TopDocsPtr topDocs = searcher->search(query, 1);
        int32_t numTotalHits = topDocs->totalHits;
        if (numTotalHits == 0) {
            if (needAdd)
                *needAdd = true;
            return true;
        }

        DocumentPtr doc = searcher->doc(topDocs->scoreDocs[0]->doc);
        auto info = InfoFactory::create<FileInfo>(QUrl::fromLocalFile(file),
                                                  Global::CreateFileInfoType::kCreateFileInfoSync);
        if (!info)
            return false;

        const QDateTime &modifyTime { info->timeOf(TimeInfoType::kLastModified).toDateTime() };
        const QString &modifyEpoch { QString::number(modifyTime.toSecsSinceEpoch()) };
        const String &storeTime { doc->get(L"modified") };

        return modifyEpoch.toStdWString() != storeTime;
    } catch (const std::exception &e) {
        fmWarning() << "Check update failed:" << file << e.what();
        return false;
    }
}

void updateFile(const QString &path, const IndexReaderPtr &reader,
                const IndexWriterPtr &writer, qint64 *processedCount)
{
    try {
        bool needAdd = false;
        if (checkNeedUpdate(path, reader, &needAdd)) {
            if (needAdd) {
                fmDebug() << "Adding new file:" << path;
                writer->addDocument(createFileDocument(path));
            } else {
                fmDebug() << "Updating file:" << path;
                TermPtr term = newLucene<Term>(L"path", path.toStdWString());
                writer->updateDocument(term, createFileDocument(path));
            }
            if (processedCount) {
                ++(*processedCount);
                emit ProgressNotifier::instance()->progressChanged(*processedCount);
            }
        }
    } catch (const std::exception &e) {
        fmWarning() << "Update file failed:" << path << e.what();
    }
}

void traverseForUpdate(const QString &rootPath, const IndexReaderPtr &reader,
                       const IndexWriterPtr &writer, qint64 *processedCount,
                       const std::atomic_bool &running)
{
    static const QRegularExpression filterRegex(kFilterFolders);
    QMap<QString, QString> bindPathTable = DeviceUtils::fstabBindInfo();

    // 用队列存储待处理的目录
    QQueue<QString> dirQueue;
    dirQueue.enqueue(rootPath);

    while (!dirQueue.isEmpty() && running.load()) {
        QString currentPath = dirQueue.dequeue();

        // 检查过滤条件
        if (bindPathTable.contains(currentPath) || (filterRegex.match(currentPath).hasMatch() && !currentPath.startsWith("/run/user")))
            continue;

        if (currentPath.size() > FILENAME_MAX - 1 || currentPath.count('/') > 20)
            continue;

        DIR *dir = opendir(currentPath.toStdString().c_str());
        if (!dir) {
            fmWarning() << "Cannot open directory:" << currentPath;
            continue;
        }

        FinallyUtil dirCloser([dir]() { closedir(dir); });

        struct dirent *entry;
        while ((entry = readdir(dir)) && running.load()) {
            if (entry->d_name[0] == '.' && strncmp(entry->d_name, ".local", strlen(".local")))
                continue;

            if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
                continue;

            QString fullPath = currentPath + "/" + entry->d_name;
            struct stat st;
            if (lstat(fullPath.toStdString().c_str(), &st) == -1)
                continue;

            if (S_ISDIR(st.st_mode)) {
                dirQueue.enqueue(fullPath);   // 将子目录加入队列
            } else {
                auto info = InfoFactory::create<FileInfo>(QUrl::fromLocalFile(fullPath),
                                                          Global::CreateFileInfoType::kCreateFileInfoSync);
                if (!info) continue;

                QString suffix = info->nameOf(NameInfoType::kSuffix);
                static const QRegularExpression suffixRegex(kSupportFiles);
                if (suffixRegex.match(suffix).hasMatch()) {
                    updateFile(fullPath, reader, writer, processedCount);
                }
            }
        }
    }
}

}   // namespace

TaskHandler TaskHandlers::CreateIndexHandler()
{
    return [](const QString &path, std::atomic_bool &running) -> bool {
        fmInfo() << "Creating index for path:" << path;

        QDir dir;
        if (!dir.exists(path)) {
            fmWarning() << "Source directory doesn't exist:" << path;
            return false;
        }

        if (!dir.exists(indexStorePath())) {
            if (!dir.mkpath(indexStorePath())) {
                fmWarning() << "Unable to create index directory:" << indexStorePath();
                return false;
            }
        }

        try {
            IndexWriterPtr writer = newLucene<IndexWriter>(
                    FSDirectory::open(indexStorePath().toStdWString()),
                    newLucene<ChineseAnalyzer>(),
                    true,
                    IndexWriter::MaxFieldLengthLIMITED);

            FinallyUtil writerCloser([&writer]() {
                try {
                    if (writer) writer->close();
                } catch (...) {}
            });

            fmInfo() << "Indexing to directory:" << indexStorePath();

            qint64 processedCount = 0;

            writer->deleteAll();
            traverseDirectory(path, writer, &processedCount, running);

            if (!running.load()) {
                fmInfo() << "Create index task was interrupted";
                return false;
            }

            writer->optimize();
            return true;
        } catch (const LuceneException &e) {
            fmWarning() << "Create index failed with Lucene exception:"
                        << QString::fromStdWString(e.getError());
        } catch (const std::exception &e) {
            fmWarning() << "Create index failed with exception:" << e.what();
        }

        return false;
    };
}

TaskHandler TaskHandlers::UpdateIndexHandler()
{
    return [](const QString &path, std::atomic_bool &running) -> bool {
        fmInfo() << "Updating index for path:" << path;

        try {
            IndexReaderPtr reader = IndexReader::open(
                    FSDirectory::open(indexStorePath().toStdWString()), true);

            FinallyUtil readerCloser([&reader]() {
                try {
                    if (reader) reader->close();
                } catch (...) {}
            });

            IndexWriterPtr writer = newLucene<IndexWriter>(
                    FSDirectory::open(indexStorePath().toStdWString()),
                    newLucene<ChineseAnalyzer>(),
                    false,
                    IndexWriter::MaxFieldLengthLIMITED);

            FinallyUtil writerCloser([&writer]() {
                try {
                    if (writer) writer->close();
                } catch (...) {}
            });

            qint64 processedCount = 0;

            traverseForUpdate(path, reader, writer, &processedCount, running);

            if (processedCount == 0) {
                emit ProgressNotifier::instance()->progressChanged(0);
            }

            writer->optimize();
            return true;
        } catch (const LuceneException &e) {
            fmWarning() << "Update index failed with Lucene exception:"
                        << QString::fromStdWString(e.getError());
        } catch (const std::exception &e) {
            fmWarning() << "Update index failed with exception:" << e.what();
        }

        return false;
    };
}
