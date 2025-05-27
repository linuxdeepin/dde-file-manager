// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "taskhandler.h"
#include "fileprovider.h"
#include "progressnotifier.h"
#include "moveprocessor.h"
#include "utils/scopeguard.h"
#include "utils/docutils.h"
#include "utils/indexutility.h"
#include "utils/textindexconfig.h"

#include <dfm-search/searchfactory.h>
#include <dfm-search/filenamesearchapi.h>

#include <fulltext/chineseanalyzer.h>

#include <lucene++/LuceneHeaders.h>
#include <FileUtils.h>
#include <FilterIndexReader.h>
#include <FuzzyQuery.h>
#include <QueryWrapperFilter.h>

#include <QDir>
#include <QDateTime>

SERVICETEXTINDEX_USE_NAMESPACE

using namespace Lucene;
DFM_SEARCH_USE_NS

namespace {

class ProgressReporter
{
public:
    explicit ProgressReporter()
        : processedCount(0), toltalCount(0), lastReportTime(QDateTime::currentDateTime())
    {
    }

    ~ProgressReporter()
    {
        // 确保最后一次进度能够显示
        emit ProgressNotifier::instance()->progressChanged(processedCount, toltalCount);
    }

    void setTotal(qint64 count)
    {
        toltalCount = count;
    }

    void increment()
    {
        ++processedCount;

        // 检查是否经过了足够的时间间隔(1秒)
        QDateTime now = QDateTime::currentDateTime();
        if (lastReportTime.msecsTo(now) >= 1000) {   // 1000ms = 1s
            emit ProgressNotifier::instance()->progressChanged(processedCount, toltalCount);
            lastReportTime = now;
        }
    }

private:
    qint64 processedCount;
    qint64 toltalCount;
    QDateTime lastReportTime;
};

// 目录遍历相关函数
using FileHandler = std::function<void(const QString &path)>;

// 文档处理相关函数
DocumentPtr createFileDocument(const QString &file)
{
    try {
        DocumentPtr doc = newLucene<Document>();

        // file path
        doc->add(newLucene<Field>(L"path", file.toStdWString(),
                                  Field::STORE_YES, Field::INDEX_NOT_ANALYZED));

        // file last modified time
        QFileInfo fileInfo(file);
        const QDateTime modifyTime = fileInfo.lastModified();
        const QString modifyEpoch = QString::number(modifyTime.toSecsSinceEpoch());
        doc->add(newLucene<Field>(L"modified", modifyEpoch.toStdWString(),
                                  Field::STORE_YES, Field::INDEX_NOT_ANALYZED));

        // file name
        doc->add(newLucene<Field>(L"filename", fileInfo.fileName().toStdWString(),
                                  Field::STORE_YES, Field::INDEX_ANALYZED));

        // hidden tag
        QString hiddenTag = "N";
        if (DFMSEARCH::Global::isHiddenPathOrInHiddenDir(fileInfo.absoluteFilePath()))
            hiddenTag = "Y";
        doc->add(newLucene<Field>(L"is_hidden", hiddenTag.toStdWString(),
                                  Field::STORE_YES, Field::INDEX_NOT_ANALYZED));

        // file contents
        const auto &contentOpt = DocUtils::extractFileContent(file);

        if (!contentOpt) {
            fmWarning() << "Failed to extract content from file:" << file;
            return doc;   // Return document without content
        }

        const QString &contents = contentOpt.value().trimmed();

        doc->add(newLucene<Field>(L"contents", contents.toStdWString(),
                                  Field::STORE_YES, Field::INDEX_ANALYZED));

        return doc;
    } catch (const LuceneException &e) {
        fmWarning() << "Create document failed with Lucene exception:" << file << QString::fromStdWString(e.getError());
    } catch (const std::exception &e) {
        fmWarning() << "Create document failed with exception:" << file << e.what();
    } catch (...) {
        fmWarning() << "Create document failed with unknown exception:" << file;
    }

    // 发生异常时返回一个空的基本文档，防止调用方受到影响
    try {
        DocumentPtr basicDoc = newLucene<Document>();
        basicDoc->add(newLucene<Field>(L"path", file.toStdWString(),
                                       Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
        return basicDoc;
    } catch (...) {
        fmWarning() << "Failed to create even a basic document for:" << file;
        return nullptr;   // 最坏情况，返回空指针，调用方需要检查
    }
}

bool checkNeedUpdate(const QString &file, const IndexReaderPtr &reader, bool *needAdd)
{
    try {
        SearcherPtr searcher = newLucene<IndexSearcher>(reader);
        TermQueryPtr query = newLucene<TermQuery>(newLucene<Term>(L"path", file.toStdWString()));

        TopDocsPtr topDocs = searcher->search(query, 1);
        int32_t numTotalHits = topDocs->totalHits;
        if (numTotalHits == 0) {
            if (needAdd)
                *needAdd = true;
            return true;
        }

        DocumentPtr doc = searcher->doc(topDocs->scoreDocs[0]->doc);
        QFileInfo fileInfo(file);
        if (!fileInfo.exists())
            return false;

        const QDateTime modifyTime = fileInfo.lastModified();
        const QString modifyEpoch = QString::number(modifyTime.toSecsSinceEpoch());
        const String &storeTime = doc->get(L"modified");

        return modifyEpoch.toStdWString() != storeTime;
    } catch (const LuceneException &e) {
        fmWarning() << "Check update failed with Lucene exception:" << file
                    << QString::fromStdWString(e.getError());
        return false;
    } catch (const std::exception &e) {
        fmWarning() << "Check update failed:" << file << e.what();
        return false;
    } catch (...) {
        fmWarning() << "Check update failed with unknown exception:" << file;
        return false;
    }
}

bool checkFileSize(const QFileInfo &fileInfo)
{
    try {
        static const qint64 kMaxFileSizeInBytes = [] {
            qint64 sizeMBFromConfig = TextIndexConfig::instance().maxIndexFileSizeMB();
            // 在这里进行上述的健全性检查
            if (sizeMBFromConfig <= 0 || sizeMBFromConfig > Q_INT64_C(0x7FFFFFFFFFFFFFFF) / (1024LL * 1024LL)) {
                sizeMBFromConfig = 50LL;   // Default fallback
            }
            return sizeMBFromConfig * 1024LL * 1024LL;
        }();

        if (fileInfo.size() > kMaxFileSizeInBytes) {
            fmDebug() << "File" << fileInfo.fileName() << "size" << fileInfo.size()
                      << "exceeds max allowed size" << kMaxFileSizeInBytes;
            return false;
        }
        return true;
    } catch (const std::exception &e) {
        fmWarning() << "Failed to check file size:" << fileInfo.filePath() << e.what();
        return false;
    } catch (...) {
        fmWarning() << "Failed to check file size with unknown exception:" << fileInfo.filePath();
        return false;
    }
}

bool isSupportedFile(const QString &path)
{
    try {
        QFileInfo fileInfo(path);
        if (!fileInfo.exists() || !fileInfo.isFile())
            return false;

        // 检查文件大小是否超过 X MB（X * 1024 * 1024 字节）
        if (!checkFileSize(fileInfo))
            return false;

        const QString &suffix = fileInfo.suffix().toLower();
        return TextIndexConfig::instance().supportedFileExtensions().contains(suffix);
    } catch (const std::exception &e) {
        fmWarning() << "Failed to check if file is supported:" << path << e.what();
        return false;
    } catch (...) {
        fmWarning() << "Failed to check if file is supported with unknown exception:" << path;
        return false;
    }
}

void processFile(const QString &path, const IndexWriterPtr &writer, ProgressReporter *reporter)
{
    try {
        if (!isSupportedFile(path))
            return;
#ifdef QT_DEBUG
        fmDebug() << "Adding [" << path << "]";
#endif
        DocumentPtr doc = createFileDocument(path);
        if (!doc) {
            fmWarning() << "Failed to create document for:" << path;
            return;
        }
        writer->addDocument(doc);
        if (reporter) {
            reporter->increment();
        }
    } catch (const LuceneException &e) {
        fmWarning() << "Process file failed with Lucene exception:" << path
                    << QString::fromStdWString(e.getError());
    } catch (const std::exception &e) {
        fmWarning() << "Process file failed:" << path << e.what();
    } catch (...) {
        fmWarning() << "Process file failed with unknown exception:" << path;
    }
}

void updateFile(const QString &path, const IndexReaderPtr &reader,
                const IndexWriterPtr &writer, ProgressReporter *reporter)
{
    try {
        if (!isSupportedFile(path))
            return;

        bool needAdd = false;
        if (checkNeedUpdate(path, reader, &needAdd)) {
            DocumentPtr doc = createFileDocument(path);
            if (!doc) {
                fmWarning() << "Failed to create document for:" << path;
                return;
            }

            if (needAdd) {
#ifdef QT_DEBUG
                fmDebug() << "Adding [" << path << "]";
#endif
                writer->addDocument(doc);
            } else {
                fmDebug() << "Updating file [" << path << "]";
                TermPtr term = newLucene<Term>(L"path", path.toStdWString());
                writer->updateDocument(term, doc);
            }
        }

        if (reporter) {
            reporter->increment();
        }
    } catch (const LuceneException &e) {
        fmWarning() << "Update file failed with Lucene exception:" << path
                    << QString::fromStdWString(e.getError());
    } catch (const std::exception &e) {
        fmWarning() << "Update file failed:" << path << e.what();
    } catch (...) {
        fmWarning() << "Update file failed with unknown exception:" << path;
    }
}

void removeFile(const QString &path, const IndexWriterPtr &writer, ProgressReporter *reporter)
{
    try {
#ifdef QT_DEBUG
        fmDebug() << "Remove [" << path << "]";
#endif
        TermPtr term = newLucene<Term>(L"path", path.toStdWString());
        writer->deleteDocuments(term);
        if (reporter) {
            reporter->increment();
        }
    } catch (const LuceneException &e) {
        fmWarning() << "Remove file failed with Lucene exception:" << path
                    << QString::fromStdWString(e.getError());
    } catch (const std::exception &e) {
        fmWarning() << "Remove file failed:" << path << e.what();
    } catch (...) {
        fmWarning() << "Remove file failed with unknown exception:" << path;
    }
}

bool cleanupIndexs(IndexReaderPtr reader, IndexWriterPtr writer, TaskState &running)
{
    try {
        if (!reader || !writer) {
            fmWarning() << "Invalid reader or writer for index cleanup";
            return false;
        }

        fmInfo() << "Checking for deleted files in index...";
        SearcherPtr searcher = newLucene<IndexSearcher>(reader);
        if (!searcher) {
            fmWarning() << "Failed to create searcher for index cleanup";
            return false;
        }

        // 获取所有文档
        TermPtr allDocsTerm = newLucene<Term>(L"path", L"*");
        WildcardQueryPtr allDocsQuery = newLucene<WildcardQuery>(allDocsTerm);
        TopDocsPtr allDocs = searcher->search(allDocsQuery, reader->maxDoc());
        if (!allDocs) {
            fmWarning() << "Search returned null result for index cleanup";
            return false;
        }

        int removedCount = 0;
        const QStringList supportedExtensions = TextIndexConfig::instance().supportedFileExtensions();
        // 检查每个文档对应的文件是否存在
        for (int32_t i = 0; i < allDocs->totalHits && running.isRunning(); ++i) {
            // Ensure scoreDocs[i] is not null before accessing ->doc
            if (!allDocs->scoreDocs || !allDocs->scoreDocs[i]) {
                fmWarning() << "Null scoreDoc at index" << i << "during index cleanup";
                return false;
            }

            DocumentPtr doc = searcher->doc(allDocs->scoreDocs[i]->doc);
            if (!doc) {   // Ensure document is valid
                fmWarning() << "Null document at index" << i << "during index cleanup";
                return false;
            }

            String pathValue = doc->get(L"path");
            if (pathValue.empty()) {
                fmWarning() << "Document at index" << i << "has empty path during index cleanup";
                return false;
            }

            QString filePath = QString::fromStdWString(pathValue);

            bool shouldDelete = false;

            QFileInfo fileInfo(filePath);

            //  Check existence first
            if (!fileInfo.exists()) {
                shouldDelete = true;
            } else {
                // If exists, check suffix (only if not already marked for deletion)
                QString suffix = fileInfo.suffix().toLower();   // Normalize to lowercase for case-insensitive comparison

                // Use the pre-fetched list/set
                // if (!supportedExtensionsSet.contains(suffix)) { // If using QSet
                if (!supportedExtensions.contains(suffix, Qt::CaseInsensitive)) {   // QStringList::contains with case insensitivity
                    shouldDelete = true;
                }
            }

            //  Delete if necessary
            if (shouldDelete) {
                try {
                    TermPtr term = newLucene<Term>(L"path", pathValue);   // Create Term only when needed
                    if (term) {
                        writer->deleteDocuments(term);
                        removedCount++;
                    }
                } catch (const std::exception &e) {
                    fmWarning() << "Failed to delete document:" << filePath << e.what();
                    // 继续处理其他文档
                } catch (...) {
                    fmWarning() << "Failed to delete document with unknown exception:" << filePath;
                    // 继续处理其他文档
                }
            }
        }

        if (removedCount > 0) {
            fmInfo() << "Removed" << removedCount << "deleted files from index";
        }

        return true;
    } catch (const LuceneException &e) {
        fmWarning() << "Error checking for deleted files with Lucene exception:"
                    << QString::fromStdWString(e.getError());
        // 继续执行，不要因为清理失败而中断整个更新过程
        return false;
    } catch (const std::exception &e) {
        fmWarning() << "Error checking for deleted files:" << e.what();
        // 继续执行，不要因为清理失败而中断整个更新过程
        return false;
    } catch (...) {
        fmWarning() << "Error checking for deleted files with unknown exception";
        // 继续执行，不要因为清理失败而中断整个更新过程
        return false;
    }
}

// 移除目录下所有文件的索引，使用前缀匹配
void removeDirectoryIndex(const QString &dirPath, const IndexWriterPtr &writer,
                          const IndexReaderPtr &reader, ProgressReporter *reporter)
{
    try {
        QString normalizedPath = dirPath;
        if (!normalizedPath.endsWith('/')) {
            normalizedPath += '/';   // 确保目录路径以/结尾，便于前缀匹配
        }
#ifdef QT_DEBUG
        fmDebug() << "Remove directory [" << normalizedPath << "]";
#endif

        // 创建前缀查询，查找所有以该目录路径开头的文档
        PrefixQueryPtr prefixQuery = newLucene<PrefixQuery>(
                newLucene<Term>(L"path", normalizedPath.toStdWString()));

        // 使用索引读取器和搜索器来找到所有匹配的文档
        SearcherPtr searcher = newLucene<IndexSearcher>(reader);
        TopDocsPtr allDocs = searcher->search(prefixQuery, reader->maxDoc());

        // 检查allDocs是否有效
        if (!allDocs) {
            fmWarning() << "Search returned null result for directory:" << dirPath;
            return;
        }

        if (allDocs->totalHits > 0)
            fmInfo() << "Found" << allDocs->totalHits << "documents to remove from directory:" << dirPath;

        // 记录已删除的文档路径以避免重复删除
        HashSet<String> pathsToDelete = HashSet<String>::newInstance();

        // 收集所有匹配的文档路径
        for (int32_t i = 0; i < allDocs->totalHits; ++i) {
            // 检查scoreDocs是否有效
            if (!allDocs->scoreDocs || !allDocs->scoreDocs[i]) {
                fmWarning() << "Null scoreDoc at index" << i << "for directory:" << dirPath;
                continue;
            }

            DocumentPtr doc = searcher->doc(allDocs->scoreDocs[i]->doc);
            if (!doc) {
                fmWarning() << "Null document at index" << i << "for directory:" << dirPath;
                continue;
            }

            String pathValue = doc->get(L"path");
            pathsToDelete.add(pathValue);
        }

        // 批量删除所有匹配的文档
        if (!pathsToDelete.empty()) {
            int deleteCount = 0;
            for (HashSet<String>::iterator it = pathsToDelete.begin();
                 it != pathsToDelete.end(); ++it) {
                TermPtr term = newLucene<Term>(L"path", *it);
                writer->deleteDocuments(term);
                deleteCount++;

                if (reporter) {
                    reporter->increment();
                }
            }
            fmInfo() << "Removed" << deleteCount << "documents from index for directory:" << dirPath;
        }
    } catch (const LuceneException &e) {
        fmWarning() << "Remove directory index failed with Lucene exception:" << dirPath
                    << QString::fromStdWString(e.getError());
    } catch (const std::exception &e) {
        fmWarning() << "Remove directory index failed:" << dirPath << e.what();
    } catch (...) {
        fmWarning() << "Remove directory index failed with unknown exception:" << dirPath;
    }
}

}   // namespace

// 创建文件提供者
std::unique_ptr<FileProvider> TaskHandlers::createFileProvider(const QString &path)
{
    if (IndexUtility::isIndexWithAnything(path)) {
        fmInfo() << "Try get docs by anything, int path: " << path;
        QObject holder;
        SearchEngine *engine = SearchFactory::createEngine(SearchType::FileName, &holder);
        SearchOptions options;
        options.setSyncSearchTimeout(120);
        // rootPath: Rely on anything's own path whitelisting mechanism to get all document paths,
        // reducing redundant operations.
        options.setSearchPath(QDir::rootPath());
        options.setSearchMethod(SearchMethod::Indexed);
        options.setIncludeHidden(TextIndexConfig::instance().indexHiddenFiles());   // Note: too many hidden files!
        FileNameOptionsAPI fileNameOptions(options);
        fileNameOptions.setFileTypes({ Defines::kAnythingDocType });
        engine->setSearchOptions(options);
        SearchQuery query = SearchFactory::createQuery("", SearchQuery::Type::Simple);
        const SearchResultExpected &result = engine->searchSync(query);
        if (result.hasValue() && !result->isEmpty()) {
            fmInfo() << "File listings are provided by ANYTHING."
                     << "count: " << result.value().count();
            return std::make_unique<DirectFileListProvider>(result.value());
        }
        fmWarning() << "Failed to get file list via ANYTHING!";
    }
    fmInfo() << "Use FileSystemProvider for: " << path;
    return std::make_unique<FileSystemProvider>(path);
}

// 创建文件列表提供者
std::unique_ptr<FileProvider> TaskHandlers::createFileListProvider(const QStringList &fileList)
{
    return std::make_unique<MixedPathListProvider>(fileList);
}

// 公开的任务处理函数实现
TaskHandler TaskHandlers::CreateIndexHandler()
{
    return [](const QString &path, TaskState &running) -> HandlerResult {
        fmInfo() << "Creating index for path:" << path;

        HandlerResult result { false, false, false };
        QDir dir;
        if (!dir.exists(path)) {
            fmWarning() << "Source directory doesn't exist:" << path;
            return result;
        }

        if (!dir.exists(DFMSEARCH::Global::contentIndexDirectory())) {
            if (!dir.mkpath(DFMSEARCH::Global::contentIndexDirectory())) {
                fmWarning() << "Unable to create index directory:" << DFMSEARCH::Global::contentIndexDirectory();
                return result;
            }
        }

        try {
            IndexWriterPtr writer = newLucene<IndexWriter>(
                    FSDirectory::open(DFMSEARCH::Global::contentIndexDirectory().toStdWString()),
                    newLucene<ChineseAnalyzer>(),
                    true,
                    IndexWriter::MaxFieldLengthUNLIMITED);

            // 添加 writer 的 ScopeGuard
            ScopeGuard writerCloser([&writer]() {
                try {
                    if (writer) writer->close();
                } catch (...) {
                    // 忽略关闭时的异常
                }
            });

            fmInfo() << "Indexing to directory:" << DFMSEARCH::Global::contentIndexDirectory();

            writer->deleteAll();

            // 使用文件提供者遍历文件
            auto provider = createFileProvider(path);
            if (!provider) {
                fmWarning() << "Failed to create file provider for path:" << path;
                return result;
            }

            if (provider->name() == "DirectFileListProvider") {
                result.useAnything = true;
            }

            ProgressReporter reporter;
            reporter.setTotal(provider->totalCount());
            provider->traverse(running, [&](const QString &file) {
                processFile(file, writer, &reporter);
            });

            // Only the creation of an index that is interrupted is also considered a failure
            // Created indexes must be guaranteed to be complete
            if (!running.isRunning()) {
                fmWarning() << "Create index task was interrupted";
                result.interrupted = true;
                result.success = false;   // 创建被打断若不失败索引是不完整的
                return result;
            }

            writer->optimize();
            result.success = true;

            return result;
        } catch (const LuceneException &e) {
            fmWarning() << "Create index failed with Lucene exception:"
                        << QString::fromStdWString(e.getError());
        } catch (const std::exception &e) {
            fmWarning() << "Create index failed with exception:" << e.what();
        }

        return result;
    };
}

TaskHandler TaskHandlers::UpdateIndexHandler()
{
    return [](const QString &path, TaskState &running) -> HandlerResult {
        fmInfo() << "Updating index for path:" << path;
        HandlerResult result { false, false, false };

        try {
            IndexReaderPtr reader = IndexReader::open(
                    FSDirectory::open(DFMSEARCH::Global::contentIndexDirectory().toStdWString()), true);

            // 添加 reader 的 ScopeGuard
            ScopeGuard readerCloser([&reader]() {
                try {
                    if (reader) reader->close();
                } catch (...) {
                    // 忽略关闭时的异常
                }
            });

            IndexWriterPtr writer = newLucene<IndexWriter>(
                    FSDirectory::open(DFMSEARCH::Global::contentIndexDirectory().toStdWString()),
                    newLucene<ChineseAnalyzer>(),
                    false,
                    IndexWriter::MaxFieldLengthUNLIMITED);

            // 添加 writer 的 ScopeGuard
            ScopeGuard writerCloser([&writer]() {
                try {
                    if (writer) writer->close();
                } catch (...) {
                    // 忽略关闭时的异常
                }
            });

            // 清理已删除文件的索引
            if (!cleanupIndexs(reader, writer, running)) {
                fmWarning() << "Index cleanup failed";
                result.success = false;
                result.fatal = true;
                return result;
            }

            // 使用文件提供者遍历文件
            auto provider = createFileProvider(path);
            if (!provider) {
                fmWarning() << "Failed to create file provider for path:" << path;
                return result;
            }

            if (provider->name() == "DirectFileListProvider") {
                result.useAnything = true;
            }

            ProgressReporter reporter;
            reporter.setTotal(provider->totalCount());
            provider->traverse(running, [&](const QString &file) {
                updateFile(file, reader, writer, &reporter);
            });

            if (!running.isRunning()) {
                fmWarning() << "Update index task was interrupted";
                result.interrupted = true;
            }

            writer->optimize();
            result.success = true;

            return result;
        } catch (const LuceneException &e) {
            // Lucene异常表示索引损坏
            fmWarning() << "Update index failed with Lucene exception, needs rebuild:"
                        << QString::fromStdWString(e.getError());
            throw;   // 重新抛出异常，让 IndexTask 捕获并处理
        } catch (const std::exception &e) {
            // 其他异常不需要重建
            fmWarning() << "Update index failed with exception:" << e.what();
        }

        return result;
    };
}

// 基于文件列表更新索引
TaskHandler TaskHandlers::CreateOrUpdateFileListHandler(const QStringList &fileList)
{
    return [fileList](const QString &path, TaskState &running) -> HandlerResult {
        Q_UNUSED(path)
        fmInfo() << "Creating/Updating index for file list with" << fileList.size() << "entries";
        HandlerResult result { false, false, false };

        try {
            IndexReaderPtr reader = IndexReader::open(
                    FSDirectory::open(DFMSEARCH::Global::contentIndexDirectory().toStdWString()), true);

            // 添加 reader 的 ScopeGuard
            ScopeGuard readerCloser([&reader]() {
                try {
                    if (reader) reader->close();
                } catch (...) {
                    // 忽略关闭时的异常
                }
            });

            IndexWriterPtr writer = newLucene<IndexWriter>(
                    FSDirectory::open(DFMSEARCH::Global::contentIndexDirectory().toStdWString()),
                    newLucene<ChineseAnalyzer>(),
                    false,
                    IndexWriter::MaxFieldLengthUNLIMITED);

            // 添加 writer 的 ScopeGuard
            ScopeGuard writerCloser([&writer]() {
                try {
                    if (writer) writer->close();
                } catch (...) {
                    // 忽略关闭时的异常
                }
            });

            // 使用文件列表提供者遍历文件
            auto provider = createFileListProvider(fileList);
            if (!provider) {
                fmWarning() << "Failed to create file list provider";
                return result;
            }

            ProgressReporter reporter;
            reporter.setTotal(provider->totalCount());
            provider->traverse(running, [&](const QString &file) {
                updateFile(file, reader, writer, &reporter);
            });

            if (!running.isRunning()) {
                fmWarning() << "Update index task was interrupted";
                result.interrupted = true;
            }

            writer->commit();
            result.success = true;

            return result;
        } catch (const LuceneException &e) {
            // Lucene异常表示索引损坏
            fmWarning() << "Update index failed with Lucene exception, needs rebuild:"
                        << QString::fromStdWString(e.getError());
            throw;   // 重新抛出异常，让 IndexTask 捕获并处理
        } catch (const std::exception &e) {
            // 其他异常不需要重建
            fmWarning() << "Update index failed with exception:" << e.what();
        }

        return result;
    };
}

// 基于文件列表删除索引
TaskHandler TaskHandlers::RemoveFileListHandler(const QStringList &fileList)
{
    return [fileList](const QString &path, TaskState &running) -> HandlerResult {
        Q_UNUSED(path)
        fmInfo() << "Removing index for" << fileList.size() << "files/directories";
        HandlerResult result { false, false, false };

        try {
            // 打开索引读取器，用于目录前缀查询
            IndexReaderPtr reader = IndexReader::open(
                    FSDirectory::open(DFMSEARCH::Global::contentIndexDirectory().toStdWString()), true);

            // 添加 reader 的 ScopeGuard
            ScopeGuard readerCloser([&reader]() {
                try {
                    if (reader) reader->close();
                } catch (...) {
                    // 忽略关闭时的异常
                }
            });

            // 打开索引写入器
            IndexWriterPtr writer = newLucene<IndexWriter>(
                    FSDirectory::open(DFMSEARCH::Global::contentIndexDirectory().toStdWString()),
                    newLucene<ChineseAnalyzer>(),
                    false,
                    IndexWriter::MaxFieldLengthUNLIMITED);

            // 添加 writer 的 ScopeGuard
            ScopeGuard writerCloser([&writer]() {
                try {
                    if (writer) writer->close();
                } catch (...) {
                    // 忽略关闭时的异常
                }
            });

            ProgressReporter reporter;

            // 直接遍历文件列表，不使用MixedPathListProvider
            for (const QString &path : fileList) {
                if (!running.isRunning())
                    break;

                // 首先尝试从索引中查找该路径
                SearcherPtr searcher = newLucene<IndexSearcher>(reader);
                TermQueryPtr pathQuery = newLucene<TermQuery>(
                        newLucene<Term>(L"path", path.toStdWString()));

                TopDocsPtr result = searcher->search(pathQuery, 1);

                if (result->totalHits > 0) {
                    // 如果找到了匹配的文档，按文件处理
                    removeFile(path, writer, &reporter);
                } else {
                    // 如果没有找到精确匹配，尝试作为目录处理
                    removeDirectoryIndex(path, writer, reader, &reporter);
                }
            }

            if (!running.isRunning()) {
                fmWarning() << "Remove index task was interrupted";
                result.interrupted = true;
            }

            writer->commit();
            result.success = true;
            return result;
        } catch (const LuceneException &e) {
            fmWarning() << "Remove index failed with Lucene exception:"
                        << QString::fromStdWString(e.getError());
        } catch (const std::exception &e) {
            fmWarning() << "Remove index failed with exception:" << e.what();
        }

        return result;
    };
}

// 基于文件移动列表高效更新索引路径
TaskHandler TaskHandlers::MoveFileListHandler(const QHash<QString, QString> &movedFiles)
{
    return [movedFiles](const QString &path, TaskState &running) -> HandlerResult {
        Q_UNUSED(path)
        fmInfo() << "Processing file moves for" << movedFiles.size() << "entries";
        HandlerResult result { false, false, false };

        try {
            IndexReaderPtr reader = IndexReader::open(
                    FSDirectory::open(DFMSEARCH::Global::contentIndexDirectory().toStdWString()), true);

            // 添加 reader 的 ScopeGuard
            ScopeGuard readerCloser([&reader]() {
                try {
                    if (reader) reader->close();
                } catch (...) {
                    // 忽略关闭时的异常
                }
            });

            IndexWriterPtr writer = newLucene<IndexWriter>(
                    FSDirectory::open(DFMSEARCH::Global::contentIndexDirectory().toStdWString()),
                    newLucene<ChineseAnalyzer>(),
                    false,
                    IndexWriter::MaxFieldLengthUNLIMITED);

            // 添加 writer 的 ScopeGuard
            ScopeGuard writerCloser([&writer]() {
                try {
                    if (writer) writer->close();
                } catch (...) {
                    // 忽略关闭时的异常
                }
            });

            ProgressReporter reporter;
            reporter.setTotal(movedFiles.size());

            SearcherPtr searcher = newLucene<IndexSearcher>(reader);

            // Create processors for different move types using the new separate classes
            FileMoveProcessor fileMoveProcessor(searcher, writer);
            DirectoryMoveProcessor directoryMoveProcessor(searcher, writer, reader);

            // Process each move operation
            for (auto it = movedFiles.constBegin(); it != movedFiles.constEnd(); ++it) {
                if (!running.isRunning()) {
                    result.interrupted = true;
                    break;
                }

                const QString &fromPath = it.key();
                const QString &toPath = it.value();

                bool success = false;
                if (PathCalculator::isDirectoryMove(toPath)) {
                    success = directoryMoveProcessor.processDirectoryMove(fromPath, toPath, running);
                } else {
                    success = fileMoveProcessor.processFileMove(fromPath, toPath);
                }

                if (!success && !running.isRunning()) {
                    result.interrupted = true;
                    break;
                }

                reporter.increment();
            }

            if (!running.isRunning()) {
                fmWarning() << "Move index task was interrupted";
                result.interrupted = true;
            }

            writer->commit();
            result.success = true;
            return result;
        } catch (const LuceneException &e) {
            fmWarning() << "Move index failed with Lucene exception:"
                        << QString::fromStdWString(e.getError());
        } catch (const std::exception &e) {
            fmWarning() << "Move index failed with exception:" << e.what();
        }

        return result;
    };
}
