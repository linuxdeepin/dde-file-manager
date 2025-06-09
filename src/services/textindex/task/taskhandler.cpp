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
        fmDebug() << "[ProgressReporter] Initialized progress reporter";
    }

    ~ProgressReporter()
    {
        // 确保最后一次进度能够显示
        emit ProgressNotifier::instance()->progressChanged(processedCount, toltalCount);
        fmDebug() << "[ProgressReporter] Final progress report - processed:" << processedCount 
                 << "total:" << toltalCount;
    }

    void setTotal(qint64 count)
    {
        toltalCount = count;
        fmInfo() << "[ProgressReporter::setTotal] Total count set to:" << count;
    }

    void increment()
    {
        ++processedCount;

        // 检查是否经过了足够的时间间隔(1秒)
        QDateTime now = QDateTime::currentDateTime();
        if (lastReportTime.msecsTo(now) >= 1000) {   // 1000ms = 1s
            emit ProgressNotifier::instance()->progressChanged(processedCount, toltalCount);
            lastReportTime = now;
            // 避免高频日志，只在特定间隔打印
            if (processedCount % 1000 == 0 || processedCount == toltalCount) {
                fmDebug() << "[ProgressReporter::increment] Progress update - processed:" << processedCount 
                         << "total:" << toltalCount;
            }
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
            fmWarning() << "[createFileDocument] Failed to extract content from file:" << file;
            return doc;   // Return document without content
        }

        const QString &contents = contentOpt.value().trimmed();

        doc->add(newLucene<Field>(L"contents", contents.toStdWString(),
                                  Field::STORE_YES, Field::INDEX_ANALYZED));

        return doc;
    } catch (const LuceneException &e) {
        fmWarning() << "[createFileDocument] Create document failed with Lucene exception:" << file 
                   << "error:" << QString::fromStdWString(e.getError());
    } catch (const std::exception &e) {
        fmWarning() << "[createFileDocument] Create document failed with exception:" << file 
                   << "error:" << e.what();
    } catch (...) {
        fmWarning() << "[createFileDocument] Create document failed with unknown exception:" << file;
    }

    // 发生异常时返回一个空的基本文档，防止调用方受到影响
    try {
        DocumentPtr basicDoc = newLucene<Document>();
        basicDoc->add(newLucene<Field>(L"path", file.toStdWString(),
                                       Field::STORE_YES, Field::INDEX_NOT_ANALYZED));
        fmDebug() << "[createFileDocument] Created basic document without content for:" << file;
        return basicDoc;
    } catch (...) {
        fmCritical() << "[createFileDocument] Failed to create even a basic document for:" << file;
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
        if (!fileInfo.exists()) {
            fmDebug() << "[checkNeedUpdate] File no longer exists:" << file;
            return false;
        }

        const QDateTime modifyTime = fileInfo.lastModified();
        const QString modifyEpoch = QString::number(modifyTime.toSecsSinceEpoch());
        const String &storeTime = doc->get(L"modified");

        bool needsUpdate = modifyEpoch.toStdWString() != storeTime;
        if (needsUpdate) {
            fmDebug() << "[checkNeedUpdate] File needs update:" << file 
                     << "stored time:" << QString::fromStdWString(storeTime) 
                     << "current time:" << modifyEpoch;
        }
        return needsUpdate;
    } catch (const LuceneException &e) {
        fmWarning() << "[checkNeedUpdate] Check update failed with Lucene exception:" << file
                    << "error:" << QString::fromStdWString(e.getError());
        return false;
    } catch (const std::exception &e) {
        fmWarning() << "[checkNeedUpdate] Check update failed with exception:" << file 
                   << "error:" << e.what();
        return false;
    } catch (...) {
        fmWarning() << "[checkNeedUpdate] Check update failed with unknown exception:" << file;
        return false;
    }
}

void processFile(const QString &path, const IndexWriterPtr &writer, ProgressReporter *reporter)
{
    try {
        if (!IndexUtility::isSupportedFile(path))
            return;
#ifdef QT_DEBUG
        fmDebug() << "Adding [" << path << "]";
#endif
        DocumentPtr doc = createFileDocument(path);
        if (!doc) {
            fmWarning() << "[processFile] Failed to create document for:" << path;
            return;
        }
        writer->addDocument(doc);
        if (reporter) {
            reporter->increment();
        }
    } catch (const LuceneException &e) {
        fmWarning() << "[processFile] Process file failed with Lucene exception:" << path
                    << "error:" << QString::fromStdWString(e.getError());
    } catch (const std::exception &e) {
        fmWarning() << "[processFile] Process file failed with exception:" << path 
                   << "error:" << e.what();
    } catch (...) {
        fmWarning() << "[processFile] Process file failed with unknown exception:" << path;
    }
}

void updateFile(const QString &path, const IndexReaderPtr &reader,
                const IndexWriterPtr &writer, ProgressReporter *reporter)
{
    try {
        if (!IndexUtility::isSupportedFile(path))
            return;

        bool needAdd = false;
        if (checkNeedUpdate(path, reader, &needAdd)) {
            DocumentPtr doc = createFileDocument(path);
            if (!doc) {
                fmWarning() << "[updateFile] Failed to create document for:" << path;
                return;
            }

            if (needAdd) {
#ifdef QT_DEBUG
                fmDebug() << "Adding [" << path << "]";
#endif
                writer->addDocument(doc);
            } else {
                fmDebug() << "[updateFile] Updating existing file:" << path;
                TermPtr term = newLucene<Term>(L"path", path.toStdWString());
                writer->updateDocument(term, doc);
            }
        }

        if (reporter) {
            reporter->increment();
        }
    } catch (const LuceneException &e) {
        fmWarning() << "[updateFile] Update file failed with Lucene exception:" << path
                    << "error:" << QString::fromStdWString(e.getError());
    } catch (const std::exception &e) {
        fmWarning() << "[updateFile] Update file failed with exception:" << path 
                   << "error:" << e.what();
    } catch (...) {
        fmWarning() << "[updateFile] Update file failed with unknown exception:" << path;
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
        fmWarning() << "[removeFile] Remove file failed with Lucene exception:" << path
                    << "error:" << QString::fromStdWString(e.getError());
    } catch (const std::exception &e) {
        fmWarning() << "[removeFile] Remove file failed with exception:" << path 
                   << "error:" << e.what();
    } catch (...) {
        fmWarning() << "[removeFile] Remove file failed with unknown exception:" << path;
    }
}

bool cleanupIndexs(IndexReaderPtr reader, IndexWriterPtr writer, TaskState &running)
{
    try {
        if (!reader || !writer) {
            fmCritical() << "[cleanupIndexs] Invalid reader or writer for index cleanup";
            return false;
        }

        fmInfo() << "[cleanupIndexs] Starting index cleanup - checking for deleted files";
        SearcherPtr searcher = newLucene<IndexSearcher>(reader);
        if (!searcher) {
            fmCritical() << "[cleanupIndexs] Failed to create searcher for index cleanup";
            return false;
        }

        // 获取所有文档
        TermPtr allDocsTerm = newLucene<Term>(L"path", L"*");
        WildcardQueryPtr allDocsQuery = newLucene<WildcardQuery>(allDocsTerm);
        TopDocsPtr allDocs = searcher->search(allDocsQuery, reader->maxDoc());
        if (!allDocs) {
            fmCritical() << "[cleanupIndexs] Search returned null result for index cleanup";
            return false;
        }

        fmInfo() << "[cleanupIndexs] Found" << allDocs->totalHits << "documents to check";

        int removedCount = 0;
        const QStringList supportedExtensions = TextIndexConfig::instance().supportedFileExtensions();
        
        // 检查每个文档对应的文件是否存在
        for (int32_t i = 0; i < allDocs->totalHits && running.isRunning(); ++i) {
            // Ensure scoreDocs[i] is not null before accessing ->doc
            if (!allDocs->scoreDocs || !allDocs->scoreDocs[i]) {
                fmWarning() << "[cleanupIndexs] Null scoreDoc at index" << i << "during index cleanup";
                return false;
            }

            DocumentPtr doc = searcher->doc(allDocs->scoreDocs[i]->doc);
            if (!doc) {   // Ensure document is valid
                fmWarning() << "[cleanupIndexs] Null document at index" << i << "during index cleanup";
                return false;
            }

            String pathValue = doc->get(L"path");
            if (pathValue.empty()) {
                fmWarning() << "[cleanupIndexs] Document at index" << i << "has empty path during index cleanup";
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
                    fmWarning() << "[cleanupIndexs] Failed to delete document:" << filePath 
                               << "error:" << e.what();
                    // 继续处理其他文档
                } catch (...) {
                    fmWarning() << "[cleanupIndexs] Failed to delete document with unknown exception:" << filePath;
                    // 继续处理其他文档
                }
            }
        }

        if (removedCount > 0) {
            fmInfo() << "[cleanupIndexs] Index cleanup completed - removed" << removedCount << "deleted/unsupported files from index";
        } else {
            fmInfo() << "[cleanupIndexs] Index cleanup completed - no files needed removal";
        }

        return true;
    } catch (const LuceneException &e) {
        fmWarning() << "[cleanupIndexs] Index cleanup failed with Lucene exception:"
                    << QString::fromStdWString(e.getError());
        // 继续执行，不要因为清理失败而中断整个更新过程
        return false;
    } catch (const std::exception &e) {
        fmWarning() << "[cleanupIndexs] Index cleanup failed with exception:" << e.what();
        // 继续执行，不要因为清理失败而中断整个更新过程
        return false;
    } catch (...) {
        fmWarning() << "[cleanupIndexs] Index cleanup failed with unknown exception";
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
        
        fmInfo() << "[removeDirectoryIndex] Removing directory index for:" << normalizedPath;

        // 创建前缀查询，查找所有以该目录路径开头的文档
        PrefixQueryPtr prefixQuery = newLucene<PrefixQuery>(
                newLucene<Term>(L"path", normalizedPath.toStdWString()));

        // 使用索引读取器和搜索器来找到所有匹配的文档
        SearcherPtr searcher = newLucene<IndexSearcher>(reader);
        TopDocsPtr allDocs = searcher->search(prefixQuery, reader->maxDoc());

        // 检查allDocs是否有效
        if (!allDocs) {
            fmWarning() << "[removeDirectoryIndex] Search returned null result for directory:" << dirPath;
            return;
        }

        if (allDocs->totalHits > 0) {
            fmInfo() << "[removeDirectoryIndex] Found" << allDocs->totalHits 
                    << "documents to remove from directory:" << dirPath;
        } else {
            fmDebug() << "[removeDirectoryIndex] No documents found for directory:" << dirPath;
            return;
        }

        // 记录已删除的文档路径以避免重复删除
        HashSet<String> pathsToDelete = HashSet<String>::newInstance();

        // 收集所有匹配的文档路径
        for (int32_t i = 0; i < allDocs->totalHits; ++i) {
            // 检查scoreDocs是否有效
            if (!allDocs->scoreDocs || !allDocs->scoreDocs[i]) {
                fmWarning() << "[removeDirectoryIndex] Null scoreDoc at index" << i << "for directory:" << dirPath;
                continue;
            }

            DocumentPtr doc = searcher->doc(allDocs->scoreDocs[i]->doc);
            if (!doc) {
                fmWarning() << "[removeDirectoryIndex] Null document at index" << i << "for directory:" << dirPath;
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
            fmInfo() << "[removeDirectoryIndex] Successfully removed" << deleteCount 
                    << "documents from index for directory:" << dirPath;
        }
    } catch (const LuceneException &e) {
        fmWarning() << "[removeDirectoryIndex] Remove directory index failed with Lucene exception:" << dirPath
                    << "error:" << QString::fromStdWString(e.getError());
    } catch (const std::exception &e) {
        fmWarning() << "[removeDirectoryIndex] Remove directory index failed with exception:" << dirPath 
                   << "error:" << e.what();
    } catch (...) {
        fmWarning() << "[removeDirectoryIndex] Remove directory index failed with unknown exception:" << dirPath;
    }
}

}   // namespace

// 创建文件提供者
std::unique_ptr<FileProvider> TaskHandlers::createFileProvider(const QString &path)
{
    if (IndexUtility::isIndexWithAnything(path)) {
        fmInfo() << "[TaskHandlers::createFileProvider] Attempting to use ANYTHING for document discovery, path:" << path;
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
            fmInfo() << "[TaskHandlers::createFileProvider] Successfully obtained file listings from ANYTHING -"
                     << "count:" << result.value().count();
            return std::make_unique<DirectFileListProvider>(result.value());
        }
        fmWarning() << "[TaskHandlers::createFileProvider] Failed to get file list via ANYTHING, falling back to filesystem provider";
    }
    fmInfo() << "[TaskHandlers::createFileProvider] Using FileSystemProvider for path:" << path;
    return std::make_unique<FileSystemProvider>(path);
}

// 创建文件列表提供者
std::unique_ptr<FileProvider> TaskHandlers::createFileListProvider(const QStringList &fileList)
{
    fmInfo() << "[TaskHandlers::createFileListProvider] Creating file list provider with" << fileList.size() << "files";
    return std::make_unique<MixedPathListProvider>(fileList);
}

// 公开的任务处理函数实现
TaskHandler TaskHandlers::CreateIndexHandler()
{
    return [](const QString &path, TaskState &running) -> HandlerResult {
        fmInfo() << "[CreateIndexHandler] Starting index creation for path:" << path;

        HandlerResult result { false, false, false };
        QDir dir;
        if (!dir.exists(path)) {
            fmCritical() << "[CreateIndexHandler] Source directory doesn't exist:" << path;
            return result;
        }

        QString indexDir = DFMSEARCH::Global::contentIndexDirectory();
        if (!dir.exists(indexDir)) {
            if (!dir.mkpath(indexDir)) {
                fmCritical() << "[CreateIndexHandler] Unable to create index directory:" << indexDir;
                return result;
            }
            fmInfo() << "[CreateIndexHandler] Created index directory:" << indexDir;
        }

        try {
            IndexWriterPtr writer = newLucene<IndexWriter>(
                    FSDirectory::open(indexDir.toStdWString()),
                    newLucene<ChineseAnalyzer>(),
                    true,
                    IndexWriter::MaxFieldLengthUNLIMITED);

            // 添加 writer 的 ScopeGuard
            ScopeGuard writerCloser([&writer]() {
                try {
                    if (writer) {
                        writer->close();
                        fmDebug() << "[CreateIndexHandler] Index writer closed successfully";
                    }
                } catch (...) {
                    fmWarning() << "[CreateIndexHandler] Exception occurred while closing index writer";
                }
            });

            fmInfo() << "[CreateIndexHandler] Index writer initialized, target directory:" << indexDir;

            writer->deleteAll();
            fmInfo() << "[CreateIndexHandler] Cleared existing index data";

            // 使用文件提供者遍历文件
            auto provider = createFileProvider(path);
            if (!provider) {
                fmCritical() << "[CreateIndexHandler] Failed to create file provider for path:" << path;
                return result;
            }

            if (provider->name() == "DirectFileListProvider") {
                result.useAnything = true;
                fmInfo() << "[CreateIndexHandler] Using ANYTHING for file discovery";
            }

            ProgressReporter reporter;
            qint64 totalCount = provider->totalCount();
            reporter.setTotal(totalCount);
            fmInfo() << "[CreateIndexHandler] Starting file processing, estimated total files:" << totalCount;

            provider->traverse(running, [&](const QString &file) {
                processFile(file, writer, &reporter);
            });

            // Only the creation of an index that is interrupted is also considered a failure
            // Created indexes must be guaranteed to be complete
            if (!running.isRunning()) {
                fmWarning() << "[CreateIndexHandler] Index creation was interrupted by user request";
                result.interrupted = true;
                result.success = false;   // 创建被打断若不失败索引是不完整的
                return result;
            }

            fmInfo() << "[CreateIndexHandler] Starting index optimization";
            writer->optimize();
            fmInfo() << "[CreateIndexHandler] Index optimization completed";
            
            result.success = true;
            fmInfo() << "[CreateIndexHandler] Index creation completed successfully for path:" << path;

            return result;
        } catch (const LuceneException &e) {
            fmCritical() << "[CreateIndexHandler] Index creation failed with Lucene exception:"
                        << QString::fromStdWString(e.getError());
        } catch (const std::exception &e) {
            fmCritical() << "[CreateIndexHandler] Index creation failed with exception:" << e.what();
        }

        return result;
    };
}

TaskHandler TaskHandlers::UpdateIndexHandler()
{
    return [](const QString &path, TaskState &running) -> HandlerResult {
        fmInfo() << "[UpdateIndexHandler] Starting index update for path:" << path;
        HandlerResult result { false, false, false };

        QString indexDir = DFMSEARCH::Global::contentIndexDirectory();

        try {
            IndexReaderPtr reader = IndexReader::open(
                    FSDirectory::open(indexDir.toStdWString()), true);

            // 添加 reader 的 ScopeGuard
            ScopeGuard readerCloser([&reader]() {
                try {
                    if (reader) {
                        reader->close();
                        fmDebug() << "[UpdateIndexHandler] Index reader closed successfully";
                    }
                } catch (...) {
                    fmWarning() << "[UpdateIndexHandler] Exception occurred while closing index reader";
                }
            });

            IndexWriterPtr writer = newLucene<IndexWriter>(
                    FSDirectory::open(indexDir.toStdWString()),
                    newLucene<ChineseAnalyzer>(),
                    false,
                    IndexWriter::MaxFieldLengthUNLIMITED);

            // 添加 writer 的 ScopeGuard
            ScopeGuard writerCloser([&writer]() {
                try {
                    if (writer) {
                        writer->close();
                        fmDebug() << "[UpdateIndexHandler] Index writer closed successfully";
                    }
                } catch (...) {
                    fmWarning() << "[UpdateIndexHandler] Exception occurred while closing index writer";
                }
            });

            fmInfo() << "[UpdateIndexHandler] Index reader and writer initialized for directory:" << indexDir;

            // 清理已删除文件的索引
            if (!cleanupIndexs(reader, writer, running)) {
                fmCritical() << "[UpdateIndexHandler] Index cleanup failed, aborting update";
                result.success = false;
                result.fatal = true;
                return result;
            }

            // 使用文件提供者遍历文件
            auto provider = createFileProvider(path);
            if (!provider) {
                fmCritical() << "[UpdateIndexHandler] Failed to create file provider for path:" << path;
                return result;
            }

            if (provider->name() == "DirectFileListProvider") {
                result.useAnything = true;
                fmInfo() << "[UpdateIndexHandler] Using ANYTHING for file discovery";
            }

            ProgressReporter reporter;
            qint64 totalCount = provider->totalCount();
            reporter.setTotal(totalCount);
            fmInfo() << "[UpdateIndexHandler] Starting file update processing, estimated total files:" << totalCount;

            provider->traverse(running, [&](const QString &file) {
                updateFile(file, reader, writer, &reporter);
            });

            if (!running.isRunning()) {
                fmWarning() << "[UpdateIndexHandler] Index update was interrupted by user request";
                result.interrupted = true;
            }

            fmInfo() << "[UpdateIndexHandler] Starting index optimization";
            writer->optimize();
            fmInfo() << "[UpdateIndexHandler] Index optimization completed";
            
            result.success = true;
            fmInfo() << "[UpdateIndexHandler] Index update completed successfully for path:" << path;

            return result;
        } catch (const LuceneException &e) {
            // Lucene异常表示索引损坏
            fmCritical() << "[UpdateIndexHandler] Index update failed with Lucene exception, index may be corrupted:"
                        << QString::fromStdWString(e.getError());
            throw;   // 重新抛出异常，让 IndexTask 捕获并处理
        } catch (const std::exception &e) {
            // 其他异常不需要重建
            fmCritical() << "[UpdateIndexHandler] Index update failed with exception:" << e.what();
        }

        return result;
    };
}

// 基于文件列表更新索引
TaskHandler TaskHandlers::CreateOrUpdateFileListHandler(const QStringList &fileList)
{
    return [fileList](const QString &path, TaskState &running) -> HandlerResult {
        Q_UNUSED(path)
        fmInfo() << "[CreateOrUpdateFileListHandler] Creating/Updating index for file list with" << fileList.size() << "entries";
        HandlerResult result { false, false, false };

        QString indexDir = DFMSEARCH::Global::contentIndexDirectory();

        try {
            IndexReaderPtr reader = IndexReader::open(
                    FSDirectory::open(indexDir.toStdWString()), true);

            // 添加 reader 的 ScopeGuard
            ScopeGuard readerCloser([&reader]() {
                try {
                    if (reader) {
                        reader->close();
                        fmDebug() << "[CreateOrUpdateFileListHandler] Index reader closed successfully";
                    }
                } catch (...) {
                    fmWarning() << "[CreateOrUpdateFileListHandler] Exception occurred while closing index reader";
                }
            });

            IndexWriterPtr writer = newLucene<IndexWriter>(
                    FSDirectory::open(indexDir.toStdWString()),
                    newLucene<ChineseAnalyzer>(),
                    false,
                    IndexWriter::MaxFieldLengthUNLIMITED);

            // 添加 writer 的 ScopeGuard
            ScopeGuard writerCloser([&writer]() {
                try {
                    if (writer) {
                        writer->close();
                        fmDebug() << "[CreateOrUpdateFileListHandler] Index writer closed successfully";
                    }
                } catch (...) {
                    fmWarning() << "[CreateOrUpdateFileListHandler] Exception occurred while closing index writer";
                }
            });

            fmInfo() << "[CreateOrUpdateFileListHandler] Index reader and writer initialized for directory:" << indexDir;

            // 使用文件列表提供者遍历文件
            auto provider = createFileListProvider(fileList);
            if (!provider) {
                fmCritical() << "[CreateOrUpdateFileListHandler] Failed to create file list provider";
                return result;
            }

            ProgressReporter reporter;
            qint64 totalCount = provider->totalCount();
            reporter.setTotal(totalCount);
            fmInfo() << "[CreateOrUpdateFileListHandler] Starting file list processing, total files:" << totalCount;

            provider->traverse(running, [&](const QString &file) {
                updateFile(file, reader, writer, &reporter);
            });

            if (!running.isRunning()) {
                fmWarning() << "[CreateOrUpdateFileListHandler] File list update was interrupted by user request";
                result.interrupted = true;
            }

            fmInfo() << "[CreateOrUpdateFileListHandler] Committing index changes";
            writer->commit();
            result.success = true;
            fmInfo() << "[CreateOrUpdateFileListHandler] File list index update completed successfully";

            return result;
        } catch (const LuceneException &e) {
            // Lucene异常表示索引损坏
            fmCritical() << "[CreateOrUpdateFileListHandler] File list update failed with Lucene exception, index may be corrupted:"
                        << QString::fromStdWString(e.getError());
            throw;   // 重新抛出异常，让 IndexTask 捕获并处理
        } catch (const std::exception &e) {
            // 其他异常不需要重建
            fmCritical() << "[CreateOrUpdateFileListHandler] File list update failed with exception:" << e.what();
        }

        return result;
    };
}

// 基于文件列表删除索引
TaskHandler TaskHandlers::RemoveFileListHandler(const QStringList &fileList)
{
    return [fileList](const QString &path, TaskState &running) -> HandlerResult {
        Q_UNUSED(path)
        fmInfo() << "[RemoveFileListHandler] Removing index for" << fileList.size() << "files/directories";
        HandlerResult result { false, false, false };

        QString indexDir = DFMSEARCH::Global::contentIndexDirectory();

        try {
            // 打开索引读取器，用于目录前缀查询
            IndexReaderPtr reader = IndexReader::open(
                    FSDirectory::open(indexDir.toStdWString()), true);

            // 添加 reader 的 ScopeGuard
            ScopeGuard readerCloser([&reader]() {
                try {
                    if (reader) {
                        reader->close();
                        fmDebug() << "[RemoveFileListHandler] Index reader closed successfully";
                    }
                } catch (...) {
                    fmWarning() << "[RemoveFileListHandler] Exception occurred while closing index reader";
                }
            });

            // 打开索引写入器
            IndexWriterPtr writer = newLucene<IndexWriter>(
                    FSDirectory::open(indexDir.toStdWString()),
                    newLucene<ChineseAnalyzer>(),
                    false,
                    IndexWriter::MaxFieldLengthUNLIMITED);

            // 添加 writer 的 ScopeGuard
            ScopeGuard writerCloser([&writer]() {
                try {
                    if (writer) {
                        writer->close();
                        fmDebug() << "[RemoveFileListHandler] Index writer closed successfully";
                    }
                } catch (...) {
                    fmWarning() << "[RemoveFileListHandler] Exception occurred while closing index writer";
                }
            });

            fmInfo() << "[RemoveFileListHandler] Index reader and writer initialized for directory:" << indexDir;

            ProgressReporter reporter;
            fmInfo() << "[RemoveFileListHandler] Starting file removal processing, total items:" << fileList.size();

            int filesRemoved = 0;
            int directoriesRemoved = 0;

            // 直接遍历文件列表，不使用MixedPathListProvider
            for (const QString &itemPath : fileList) {
                if (!running.isRunning()) {
                    fmInfo() << "[RemoveFileListHandler] File removal was interrupted by user request";
                    break;
                }

                // 首先尝试从索引中查找该路径
                SearcherPtr searcher = newLucene<IndexSearcher>(reader);
                TermQueryPtr pathQuery = newLucene<TermQuery>(
                        newLucene<Term>(L"path", itemPath.toStdWString()));

                TopDocsPtr searchResult = searcher->search(pathQuery, 1);

                if (searchResult->totalHits > 0) {
                    // 如果找到了匹配的文档，按文件处理
                    removeFile(itemPath, writer, &reporter);
                    filesRemoved++;
                    fmDebug() << "[RemoveFileListHandler] Removed file from index:" << itemPath;
                } else {
                    // 如果没有找到精确匹配，尝试作为目录处理
                    removeDirectoryIndex(itemPath, writer, reader, &reporter);
                    directoriesRemoved++;
                    fmDebug() << "[RemoveFileListHandler] Processed directory removal:" << itemPath;
                }
            }

            if (!running.isRunning()) {
                fmWarning() << "[RemoveFileListHandler] File removal task was interrupted by user request";
                result.interrupted = true;
            }

            fmInfo() << "[RemoveFileListHandler] Committing index changes";
            writer->commit();
            result.success = true;
            fmInfo() << "[RemoveFileListHandler] File removal completed successfully - files:" << filesRemoved 
                    << "directories:" << directoriesRemoved;
            return result;
        } catch (const LuceneException &e) {
            fmCritical() << "[RemoveFileListHandler] File removal failed with Lucene exception:"
                        << QString::fromStdWString(e.getError());
        } catch (const std::exception &e) {
            fmCritical() << "[RemoveFileListHandler] File removal failed with exception:" << e.what();
        }

        return result;
    };
}

// 基于文件移动列表高效更新索引路径
TaskHandler TaskHandlers::MoveFileListHandler(const QHash<QString, QString> &movedFiles)
{
    return [movedFiles](const QString &path, TaskState &running) -> HandlerResult {
        Q_UNUSED(path)
        fmInfo() << "[MoveFileListHandler] Processing file moves for" << movedFiles.size() << "entries";
        HandlerResult result { false, false, false };

        QString indexDir = DFMSEARCH::Global::contentIndexDirectory();

        try {
            IndexReaderPtr reader = IndexReader::open(
                    FSDirectory::open(indexDir.toStdWString()), true);

            // 添加 reader 的 ScopeGuard
            ScopeGuard readerCloser([&reader]() {
                try {
                    if (reader) {
                        reader->close();
                        fmDebug() << "[MoveFileListHandler] Index reader closed successfully";
                    }
                } catch (...) {
                    fmWarning() << "[MoveFileListHandler] Exception occurred while closing index reader";
                }
            });

            IndexWriterPtr writer = newLucene<IndexWriter>(
                    FSDirectory::open(indexDir.toStdWString()),
                    newLucene<ChineseAnalyzer>(),
                    false,
                    IndexWriter::MaxFieldLengthUNLIMITED);

            // 添加 writer 的 ScopeGuard
            ScopeGuard writerCloser([&writer]() {
                try {
                    if (writer) {
                        writer->close();
                        fmDebug() << "[MoveFileListHandler] Index writer closed successfully";
                    }
                } catch (...) {
                    fmWarning() << "[MoveFileListHandler] Exception occurred while closing index writer";
                }
            });

            fmInfo() << "[MoveFileListHandler] Index reader and writer initialized for directory:" << indexDir;

            ProgressReporter reporter;
            reporter.setTotal(movedFiles.size());
            fmInfo() << "[MoveFileListHandler] Starting file move processing, total moves:" << movedFiles.size();

            SearcherPtr searcher = newLucene<IndexSearcher>(reader);

            // Create processors for different move types using the new separate classes
            FileMoveProcessor fileMoveProcessor(searcher, writer);
            DirectoryMoveProcessor directoryMoveProcessor(searcher, writer, reader);

            int fileMoves = 0;
            int directoryMoves = 0;
            int failedMoves = 0;

            // Process each move operation
            for (auto it = movedFiles.constBegin(); it != movedFiles.constEnd(); ++it) {
                if (!running.isRunning()) {
                    fmInfo() << "[MoveFileListHandler] File move processing was interrupted by user request";
                    result.interrupted = true;
                    break;
                }

                const QString &fromPath = it.key();
                const QString &toPath = it.value();

                bool success = false;
                if (PathCalculator::isDirectoryMove(toPath)) {
                    success = directoryMoveProcessor.processDirectoryMove(fromPath, toPath, running);
                    if (success) {
                        directoryMoves++;
                    } else {
                        failedMoves++;
                    }
                } else {
                    success = fileMoveProcessor.processFileMove(fromPath, toPath);
                    if (success) {
                        fileMoves++;
                    } else {
                        failedMoves++;
                    }
                }

                if (!success && !running.isRunning()) {
                    result.interrupted = true;
                    break;
                }

                reporter.increment();
            }

            if (!running.isRunning()) {
                fmWarning() << "[MoveFileListHandler] File move task was interrupted by user request";
                result.interrupted = true;
            }

            fmInfo() << "[MoveFileListHandler] Committing index changes";
            writer->commit();
            result.success = true;
            fmInfo() << "[MoveFileListHandler] File move processing completed successfully - file moves:" << fileMoves 
                    << "directory moves:" << directoryMoves << "failed moves:" << failedMoves;
            return result;
        } catch (const LuceneException &e) {
            fmCritical() << "[MoveFileListHandler] File move processing failed with Lucene exception:"
                        << QString::fromStdWString(e.getError());
        } catch (const std::exception &e) {
            fmCritical() << "[MoveFileListHandler] File move processing failed with exception:" << e.what();
        }

        return result;
    };
}
