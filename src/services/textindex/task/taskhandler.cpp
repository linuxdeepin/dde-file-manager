// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "taskhandler.h"
#include "fileprovider.h"
#include "progressnotifier.h"
#include "moveprocessor.h"
#include "utils/scopeguard.h"
#include "utils/indexutility.h"
#include "utils/textindexconfig.h"
#include "utils/pathexcludematcher.h"

#include <dfm-search/searchfactory.h>
#include <dfm-search/filenamesearchapi.h>
#include <dfm-search/field_names.h>

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
using namespace DFMSEARCH::LuceneFieldNames;
namespace {

std::unique_ptr<FileProvider> createAnythingFileProvider(const IndexContext &context, const QString &path)
{
    if (!IndexUtility::isIndexWithAnything(path) || !context.profile().supportsAnything()) {
        return nullptr;
    }

    const auto anythingOptions = context.profile().anythingSearchOptions();

    fmDebug() << "[TaskHandlers::createAnythingFileProvider] Attempting to use ANYTHING for profile:"
              << context.profile().id() << "path:" << path
              << "file types:" << anythingOptions.fileTypes
              << "file extensions:" << anythingOptions.fileExtensions;

    QObject holder;
    SearchEngine *engine = SearchFactory::createEngine(SearchType::FileName, &holder);
    if (!engine) {
        fmWarning() << "[TaskHandlers::createAnythingFileProvider] Failed to create ANYTHING search engine for profile:"
                    << context.profile().id();
        return nullptr;
    }

    SearchOptions options;
    options.setSyncSearchTimeout(120);
    options.setSearchPath(QDir::rootPath());
    options.setSearchMethod(SearchMethod::Indexed);
    options.setIncludeHidden(TextIndexConfig::instance().indexHiddenFiles());

    FileNameOptionsAPI fileNameOptions(options);
    if (!anythingOptions.fileTypes.isEmpty()) {
        fileNameOptions.setFileTypes(anythingOptions.fileTypes);
    }
    if (!anythingOptions.fileExtensions.isEmpty()) {
        fileNameOptions.setFileExtensions(anythingOptions.fileExtensions);
    }
    engine->setSearchOptions(options);

    SearchQuery query = SearchFactory::createQuery("", SearchQuery::Type::Simple);
    const SearchResultExpected &result = engine->searchSync(query);
    if (!result.hasValue() || result->isEmpty()) {
        fmWarning() << "[TaskHandlers::createAnythingFileProvider] ANYTHING returned no results for profile:"
                    << context.profile().id();
        return nullptr;
    }

    fmInfo() << "[TaskHandlers::createAnythingFileProvider] Successfully obtained file listings from ANYTHING for profile:"
             << context.profile().id() << "count:" << result.value().count();
    return std::make_unique<DirectFileListProvider>(result.value());
}   // namespace

class ProgressReporter
{
public:
    explicit ProgressReporter(IndexWriterPtr writer = nullptr)
        : processedCount(0), totalCount(0), lastReportTime(QDateTime::currentDateTime()), m_writer(writer), m_batchCommitInterval(TextIndexConfig::instance().batchCommitInterval()), m_lastCommitCount(0)
    {
        fmDebug() << "[ProgressReporter] Initialized progress reporter with batch commit interval:" << m_batchCommitInterval;
    }

    ~ProgressReporter()
    {
        // 确保最后一次进度能够显示
        emit ProgressNotifier::instance()->progressChanged(processedCount, totalCount);
        fmDebug() << "[ProgressReporter] Final progress report - processed:" << processedCount
                  << "total:" << totalCount << "indexChanged:" << m_indexChanged;

        // 如果有未提交的更改，进行最后一次提交
        if (m_writer && processedCount > m_lastCommitCount) {
            try {
                m_writer->commit();
                fmDebug() << "[ProgressReporter] Final batch commit completed - processed:" << processedCount;
            } catch (const std::exception &e) {
                fmWarning() << "[ProgressReporter] Final batch commit failed:" << e.what();
            } catch (...) {
                fmWarning() << "[ProgressReporter] Final batch commit failed with unknown exception";
            }
        }
    }

    void setTotal(qint64 count)
    {
        totalCount = count;
        fmInfo() << "[ProgressReporter::setTotal] Total count set to:" << count;
    }

    void increment()
    {
        ++processedCount;

        // 检查是否需要批量提交
        if (m_writer && (processedCount - m_lastCommitCount) >= m_batchCommitInterval) {
            try {
                m_writer->commit();
                m_lastCommitCount = processedCount;
                fmDebug() << "[ProgressReporter::increment] Batch commit completed at count:" << processedCount;
            } catch (const std::exception &e) {
                fmWarning() << "[ProgressReporter::increment] Batch commit failed at count:" << processedCount
                            << "error:" << e.what();
            } catch (...) {
                fmWarning() << "[ProgressReporter::increment] Batch commit failed with unknown exception at count:" << processedCount;
            }
        }

        // 检查是否经过了足够的时间间隔(1秒)
        QDateTime now = QDateTime::currentDateTime();
        if (lastReportTime.msecsTo(now) >= 1000) {   // 1000ms = 1s
            emit ProgressNotifier::instance()->progressChanged(processedCount, totalCount);
            lastReportTime = now;
            // 避免高频日志，只在特定间隔打印
            if (processedCount % 1000 == 0 || processedCount == totalCount) {
                fmDebug() << "[ProgressReporter::increment] Progress update - processed:" << processedCount
                          << "total:" << totalCount;
            }
        }
    }

    void markIndexChanged()
    {
        m_indexChanged = true;
    }

    bool indexChanged() const
    {
        return m_indexChanged;
    }

private:
    qint64 processedCount;
    qint64 totalCount;
    QDateTime lastReportTime;
    IndexWriterPtr m_writer;
    int m_batchCommitInterval;
    qint64 m_lastCommitCount;
    bool m_indexChanged { false };
};

// 目录遍历相关函数
using FileHandler = std::function<void(const QString &path)>;

const wchar_t *pathField(const IndexProfile &profile)
{
    switch (profile.type()) {
    case IndexProfile::Type::Ocr:
        return OcrText::kPath;
    case IndexProfile::Type::Content:
    default:
        return Content::kPath;
    }
}

const wchar_t *ancestorPathsField(const IndexProfile &profile)
{
    switch (profile.type()) {
    case IndexProfile::Type::Ocr:
        return OcrText::kAncestorPaths;
    case IndexProfile::Type::Content:
    default:
        return Content::kAncestorPaths;
    }
}

bool supportsModifiedTimestampCheck(const IndexProfile &profile)
{
    switch (profile.type()) {
    case IndexProfile::Type::Ocr:
    case IndexProfile::Type::Content:
        return true;
    default:
        return false;
    }
}

const wchar_t *modifyTimeField(const IndexProfile &profile)
{
    switch (profile.type()) {
    case IndexProfile::Type::Ocr:
        return OcrText::kModifyTime;
    case IndexProfile::Type::Content:
    default:
        return Content::kModifyTime;
    }
}

DocumentPtr createFileDocument(const IndexContext &context, const QString &file)
{
    try {
        if (!context.extractor() || !context.documentBuilder()) {
            fmCritical() << "[createFileDocument] Missing extractor or document builder for profile:" << context.profile().id();
            return nullptr;
        }

        const TextIndexConfig &config = TextIndexConfig::instance();
        const int truncationSizeMB = context.profile().type() == IndexProfile::Type::Ocr
                ? config.maxOcrImageSizeMB()
                : config.maxIndexFileTruncationSizeMB();
        const size_t maxBytes = static_cast<size_t>(truncationSizeMB) * 1024 * 1024;

        const IndexExtractionResult extraction = context.extractor()->extract(file, maxBytes);
        if (!extraction.success) {
            fmInfo() << "[createFileDocument] Failed to extract content from file:" << file
                     << "profile:" << context.profile().id()
                     << "error:" << extraction.error;
        }

        return context.documentBuilder()->build(file, extraction.text);
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
        DocumentPtr basicDoc = context.documentBuilder()->build(file, QString());
        fmDebug() << "[createFileDocument] Created basic document without content for:" << file;
        return basicDoc;
    } catch (...) {
        fmCritical() << "[createFileDocument] Failed to create even a basic document for:" << file;
        return nullptr;   // 最坏情况，返回空指针，调用方需要检查
    }
}

bool checkNeedUpdate(const IndexContext &context, const QString &file, const IndexReaderPtr &reader, bool *needAdd)
{
    try {
        SearcherPtr searcher = newLucene<IndexSearcher>(reader);
        TermQueryPtr query = newLucene<TermQuery>(newLucene<Term>(pathField(context.profile()), file.toStdWString()));

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

        if (!supportsModifiedTimestampCheck(context.profile())) {
            return true;
        }

        const QDateTime modifyTime = fileInfo.lastModified();
        const QString modifyEpoch = QString::number(modifyTime.toSecsSinceEpoch());
        const String &storeTime = doc->get(modifyTimeField(context.profile()));

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

bool shouldSkipExcludedFile(const QString &path, const PathExcludeMatcher &excludeMatcher)
{
    const QFileInfo fileInfo(path);
    const QString fileDir = fileInfo.absolutePath();
    if (excludeMatcher.shouldExclude(fileDir)) {
        fmDebug() << "[shouldSkipExcludedFile] Skipping blacklisted file:" << path
                  << "(directory:" << fileDir << "matches blacklist)";
        return true;
    }

    return false;
}

void processFile(const IndexContext &context, const QString &path, const PathExcludeMatcher &excludeMatcher,
                 const IndexWriterPtr &writer, ProgressReporter *reporter)
{
    try {
        if (!context.profile().isCandidateFile(path))
            return;
        if (shouldSkipExcludedFile(path, excludeMatcher))
            return;
#ifdef QT_DEBUG
        fmDebug() << "Adding [" << path << "]";
#endif
        DocumentPtr doc = createFileDocument(context, path);
        if (!doc) {
            fmWarning() << "[processFile] Failed to create document for:" << path;
            return;
        }
        writer->addDocument(doc);
        if (reporter) {
            reporter->markIndexChanged();
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

void updateFile(const IndexContext &context, const QString &path, const PathExcludeMatcher &excludeMatcher,
                const IndexReaderPtr &reader,
                const IndexWriterPtr &writer, ProgressReporter *reporter)
{
    try {
        if (!context.profile().isCandidateFile(path))
            return;
        if (shouldSkipExcludedFile(path, excludeMatcher))
            return;

        bool needAdd = false;
        if (checkNeedUpdate(context, path, reader, &needAdd)) {
            DocumentPtr doc = createFileDocument(context, path);
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
                TermPtr term = newLucene<Term>(pathField(context.profile()), path.toStdWString());
                writer->updateDocument(term, doc);
            }

            if (reporter) {
                reporter->markIndexChanged();
                reporter->increment();
            }
        } else {
            if (reporter) {
                reporter->increment();
            }
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

void removeFile(const IndexContext &context, const QString &path, const IndexWriterPtr &writer, ProgressReporter *reporter)
{
    try {
#ifdef QT_DEBUG
        fmDebug() << "Remove [" << path << "]";
#endif
        TermPtr term = newLucene<Term>(pathField(context.profile()), path.toStdWString());
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

bool cleanupIndexs(const IndexContext &context, IndexReaderPtr reader, IndexWriterPtr writer, TaskState &running, ProgressReporter *reporter)
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
        TermPtr allDocsTerm = newLucene<Term>(pathField(context.profile()), L"*");
        WildcardQueryPtr allDocsQuery = newLucene<WildcardQuery>(allDocsTerm);
        TopDocsPtr allDocs = searcher->search(allDocsQuery, reader->maxDoc());
        if (!allDocs) {
            fmCritical() << "[cleanupIndexs] Search returned null result for index cleanup";
            return false;
        }

        fmInfo() << "[cleanupIndexs] Found" << allDocs->totalHits << "documents to check";

        // Use static factory method to create configured blacklist matcher
        PathExcludeMatcher excludeMatcher = PathExcludeMatcher::createForIndex();
        fmDebug() << "[cleanupIndexs] Initialized with" << excludeMatcher.patternCount()
                  << "blacklist patterns";

        int removedCount = 0;

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

            String pathValue = doc->get(pathField(context.profile()));
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
                if (!context.profile().isPathInScope(filePath) || !context.profile().isCandidateFile(filePath)) {
                    shouldDelete = true;
                }

                // Check blacklist (only for existing and supported files)
                // Note: Blacklist rules target directories, so extract file's directory for matching
                if (!shouldDelete) {
                    const QString &fileDir = fileInfo.absolutePath();
                    if (excludeMatcher.shouldExclude(fileDir)) {
                        fmDebug() << "[cleanupIndexs] Removing blacklisted file from index:" << filePath
                                  << "(directory:" << fileDir << "matches blacklist)";
                        shouldDelete = true;
                    }
                }
            }

            //  Delete if necessary
            if (shouldDelete) {
                try {
                    TermPtr term = newLucene<Term>(pathField(context.profile()), pathValue);   // Create Term only when needed
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
            fmInfo() << "[cleanupIndexs] Index cleanup completed - removed" << removedCount << "deleted/unsupported/blacklisted files from index";
            if (reporter) {
                reporter->markIndexChanged();
            }
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

// 移除目录下所有文件的索引
void removeDirectoryIndex(const IndexContext &context, const QString &dirPath, const IndexWriterPtr &writer,
                          const IndexReaderPtr &reader, ProgressReporter *reporter)
{
    try {
        fmInfo() << "[removeDirectoryIndex] Removing directory index for:" << dirPath;

        SearcherPtr searcher = newLucene<IndexSearcher>(reader);

        // 使用 TermQuery 在 ancestor_paths 字段上进行精确匹配
        // ancestor_paths 字段存储了文件的所有祖先路径（不带尾部斜杠）
        // 利用此字段可以避免 PrefixQuery 的字典树扫描，显著提升性能
        TermQueryPtr ancestorQuery = newLucene<TermQuery>(
                newLucene<Term>(ancestorPathsField(context.profile()), dirPath.toStdWString()));

        TopDocsPtr allDocs = searcher->search(ancestorQuery, reader->maxDoc());

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

        // 直接批量删除所有匹配的文档
        // writer->deleteDocuments(Query) 会删除所有匹配该查询的文档
        // 相比逐个删除，这种方式性能更高且原子性更好
        int32_t deleteCount = allDocs->totalHits;
        writer->deleteDocuments(ancestorQuery);

        if (reporter) {
            reporter->markIndexChanged();
        }

        // 更新进度报告
        if (reporter) {
            for (int32_t i = 0; i < deleteCount; ++i) {
                reporter->increment();
            }
        }

        fmDebug() << "[removeDirectoryIndex] Successfully removed" << deleteCount
                  << "documents from index for directory:" << dirPath;
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
std::unique_ptr<FileProvider> TaskHandlers::createFileProvider(const IndexContext &context, const QString &path)
{
    if (auto provider = createAnythingFileProvider(context, path)) {
        return provider;
    }

    fmInfo() << "[TaskHandlers::createFileProvider] Using FileSystemProvider for path:" << path;
    return std::make_unique<FileSystemProvider>(context.profile(), path);
}

// 创建文件列表提供者
std::unique_ptr<FileProvider> TaskHandlers::createFileListProvider(const IndexContext &context, const QStringList &fileList)
{
    fmInfo() << "[TaskHandlers::createFileListProvider] Creating file list provider with" << fileList.size() << "files";
    return std::make_unique<MixedPathListProvider>(context.profile(), fileList);
}

// 公开的任务处理函数实现
TaskHandler TaskHandlers::CreateIndexHandler(const IndexContext &context)
{
    return [context](const QString &path, TaskState &running) -> HandlerResult {
        fmInfo() << "[CreateIndexHandler] Starting index creation for path:" << path;

        HandlerResult result { false, false, false };
        QDir dir;
        if (!dir.exists(path)) {
            fmCritical() << "[CreateIndexHandler] Source directory doesn't exist:" << path;
            return result;
        }

        QString indexDir = context.profile().indexDirectory();
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
            auto provider = createFileProvider(context, path);
            if (!provider) {
                fmCritical() << "[CreateIndexHandler] Failed to create file provider for path:" << path;
                return result;
            }

            if (provider->name() == "DirectFileListProvider") {
                result.useAnything = true;
                fmInfo() << "[CreateIndexHandler] Using ANYTHING for file discovery";
            }

            ProgressReporter reporter(writer);
            const PathExcludeMatcher excludeMatcher = PathExcludeMatcher::createForIndex();
            qint64 totalCount = provider->totalCount();
            reporter.setTotal(totalCount);
            fmInfo() << "[CreateIndexHandler] Starting file processing, estimated total files:" << totalCount;

            provider->traverse(running, [&](const QString &file) {
                processFile(context, file, excludeMatcher, writer, &reporter);
            });

            // Only the creation of an index that is interrupted is also considered a failure
            // Created indexes must be guaranteed to be complete
            if (!running.isRunning()) {
                fmWarning() << "[CreateIndexHandler] Index creation was interrupted by user request";
                result.interrupted = true;
                result.success = false;   // 创建被打断若不失败索引是不完整的
                return result;
            }

            // ProgressReporter的析构函数会处理最后的commit，但为了确保在optimize前所有更改都已提交
            // 我们显式调用一次commit
            fmDebug() << "[CreateIndexHandler] Ensuring all changes are committed before optimization";
            writer->commit();

            fmDebug() << "[CreateIndexHandler] Starting index optimization";
            writer->optimize();
            fmDebug() << "[CreateIndexHandler] Index optimization completed";

            result.success = true;
            result.indexChanged = reporter.indexChanged();
            fmDebug() << "[CreateIndexHandler] Index creation completed successfully for path:" << path;

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

TaskHandler TaskHandlers::UpdateIndexHandler(const IndexContext &context)
{
    return [context](const QString &path, TaskState &running) -> HandlerResult {
        fmInfo() << "[UpdateIndexHandler] Starting index update for path:" << path;
        HandlerResult result { false, false, false };

        QString indexDir = context.profile().indexDirectory();

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

            fmDebug() << "[UpdateIndexHandler] Index reader and writer initialized for directory:" << indexDir;

            ProgressReporter reporter(writer);

            // 清理已删除文件的索引
            if (!cleanupIndexs(context, reader, writer, running, &reporter)) {
                fmCritical() << "[UpdateIndexHandler] Index cleanup failed, aborting update";
                result.success = false;
                result.fatal = true;
                return result;
            }

            // 使用文件提供者遍历文件
            auto provider = createFileProvider(context, path);
            if (!provider) {
                fmCritical() << "[UpdateIndexHandler] Failed to create file provider for path:" << path;
                return result;
            }

            if (provider->name() == "DirectFileListProvider") {
                result.useAnything = true;
                fmInfo() << "[UpdateIndexHandler] Using ANYTHING for file discovery";
            }

            const PathExcludeMatcher excludeMatcher = PathExcludeMatcher::createForIndex();
            qint64 totalCount = provider->totalCount();
            reporter.setTotal(totalCount);
            fmDebug() << "[UpdateIndexHandler] Starting file update processing, estimated total files:" << totalCount;

            provider->traverse(running, [&](const QString &file) {
                updateFile(context, file, excludeMatcher, reader, writer, &reporter);
            });

            if (!running.isRunning()) {
                fmWarning() << "[UpdateIndexHandler] Index update was interrupted by user request";
                result.interrupted = true;
            }

            // ProgressReporter的析构函数会处理最后的commit，但为了确保在optimize前所有更改都已提交
            // 我们显式调用一次commit
            fmDebug() << "[UpdateIndexHandler] Ensuring all changes are committed before optimization";
            writer->commit();

            fmDebug() << "[UpdateIndexHandler] Starting index optimization";
            writer->optimize();
            fmDebug() << "[UpdateIndexHandler] Index optimization completed";

            result.success = true;
            result.indexChanged = reporter.indexChanged();
            fmDebug() << "[UpdateIndexHandler] Index update completed successfully for path:" << path;

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
TaskHandler TaskHandlers::CreateOrUpdateFileListHandler(const IndexContext &context, const QStringList &fileList)
{
    return [context, fileList](const QString &path, TaskState &running) -> HandlerResult {
        Q_UNUSED(path)
        fmInfo() << "[CreateOrUpdateFileListHandler] Creating/Updating index for file list with" << fileList.size() << "entries";
        HandlerResult result { false, false, false };

        QString indexDir = context.profile().indexDirectory();

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

            fmDebug() << "[CreateOrUpdateFileListHandler] Index reader and writer initialized for directory:" << indexDir;

            // 使用文件列表提供者遍历文件
            auto provider = createFileListProvider(context, fileList);
            if (!provider) {
                fmCritical() << "[CreateOrUpdateFileListHandler] Failed to create file list provider";
                return result;
            }

            ProgressReporter reporter(writer);
            const PathExcludeMatcher excludeMatcher = PathExcludeMatcher::createForIndex();
            qint64 totalCount = provider->totalCount();
            reporter.setTotal(totalCount);
            fmInfo() << "[CreateOrUpdateFileListHandler] Starting file list processing, total files:" << totalCount;

            provider->traverse(running, [&](const QString &file) {
                updateFile(context, file, excludeMatcher, reader, writer, &reporter);
            });

            if (!running.isRunning()) {
                fmWarning() << "[CreateOrUpdateFileListHandler] File list update was interrupted by user request";
                result.interrupted = true;
            }

            // ProgressReporter的析构函数会处理最后的commit
            result.success = true;
            result.indexChanged = reporter.indexChanged();
            fmDebug() << "[CreateOrUpdateFileListHandler] File list index update completed successfully";

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
TaskHandler TaskHandlers::RemoveFileListHandler(const IndexContext &context, const QStringList &fileList)
{
    return [context, fileList](const QString &path, TaskState &running) -> HandlerResult {
        Q_UNUSED(path)
        fmInfo() << "[RemoveFileListHandler] Removing index for" << fileList.size() << "files/directories";
        HandlerResult result { false, false, false };

        QString indexDir = context.profile().indexDirectory();

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

            fmDebug() << "[RemoveFileListHandler] Index reader and writer initialized for directory:" << indexDir;

            ProgressReporter reporter(writer);
            fmInfo() << "[RemoveFileListHandler] Starting file removal processing, total items:" << fileList.size();

            SearcherPtr searcher = newLucene<IndexSearcher>(reader);

            int filesRemoved = 0;
            int directoriesRemoved = 0;

            // 遍历文件列表，通过索引查询判断是文件还是目录
            for (const QString &itemPath : fileList) {
                if (!running.isRunning()) {
                    fmInfo() << "[RemoveFileListHandler] File removal was interrupted by user request";
                    break;
                }

                // 通过 ancestor_paths 查询判断是否为目录
                TermQueryPtr ancestorQuery = newLucene<TermQuery>(
                        newLucene<Term>(ancestorPathsField(context.profile()), itemPath.toStdWString()));
                TopDocsPtr result = searcher->search(ancestorQuery, 1);

                if (result->totalHits > 0) {
                    // 有子文件，是目录
                    removeDirectoryIndex(context, itemPath, writer, reader, &reporter);
                    directoriesRemoved++;
                    fmDebug() << "[RemoveFileListHandler] Processed directory removal:" << itemPath;
                } else {
                    // 无子文件，直接删除；deleteDocuments 是幂等的，不存在的路径是安全 no-op
                    removeFile(context, itemPath, writer, &reporter);
                    filesRemoved++;
                    fmDebug() << "[RemoveFileListHandler] Removed file from index:" << itemPath;
                }
            }

            if (!running.isRunning()) {
                fmWarning() << "[RemoveFileListHandler] File removal task was interrupted by user request";
                result.interrupted = true;
            }

            // ProgressReporter的析构函数会处理最后的commit
            result.success = true;
            result.indexChanged = reporter.indexChanged();
            fmDebug() << "[RemoveFileListHandler] File removal completed successfully - files:" << filesRemoved
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
TaskHandler TaskHandlers::MoveFileListHandler(const IndexContext &context, const QHash<QString, QString> &movedFiles)
{
    return [context, movedFiles](const QString &path, TaskState &running) -> HandlerResult {
        Q_UNUSED(path)
        fmInfo() << "[MoveFileListHandler] Processing file moves for" << movedFiles.size() << "entries";
        HandlerResult result { false, false, false };

        QString indexDir = context.profile().indexDirectory();

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

            fmDebug() << "[MoveFileListHandler] Index reader and writer initialized for directory:" << indexDir;

            ProgressReporter reporter(writer);
            reporter.setTotal(movedFiles.size());
            fmInfo() << "[MoveFileListHandler] Starting file move processing, total moves:" << movedFiles.size();

            SearcherPtr searcher = newLucene<IndexSearcher>(reader);

            // Create processors for different move types using the new separate classes
            FileMoveProcessor fileMoveProcessor(context, searcher, writer);
            DirectoryMoveProcessor directoryMoveProcessor(context, searcher, writer, reader);

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
                        if (directoryMoveProcessor.hasChanges())
                            reporter.markIndexChanged();
                    } else {
                        failedMoves++;
                    }
                } else {
                    success = fileMoveProcessor.processFileMove(fromPath, toPath);
                    if (success) {
                        fileMoves++;
                        if (fileMoveProcessor.hasChanges())
                            reporter.markIndexChanged();
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

            // ProgressReporter的析构函数会处理最后的commit
            result.success = true;
            result.indexChanged = reporter.indexChanged();
            fmDebug() << "[MoveFileListHandler] File move processing completed successfully - file moves:" << fileMoves
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
