// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "taskhandler.h"
#include "fileprovider.h"
#include "progressnotifier.h"
#include "utils/scopeguard.h"
#include "utils/docutils.h"
#include "utils/indexutility.h"

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
        : processedCount(0), lastReportTime(QDateTime::currentDateTime())
    {
    }

    ~ProgressReporter()
    {
        // 确保最后一次进度能够显示
        emit ProgressNotifier::instance()->progressChanged(processedCount);
    }

    void increment()
    {
        ++processedCount;

        // 检查是否经过了足够的时间间隔(1秒)
        QDateTime now = QDateTime::currentDateTime();
        if (lastReportTime.msecsTo(now) >= 1000) {   // 1000ms = 1s
            emit ProgressNotifier::instance()->progressChanged(processedCount);
            lastReportTime = now;
        }
    }

private:
    qint64 processedCount;
    QDateTime lastReportTime;
};

// 目录遍历相关函数
using FileHandler = std::function<void(const QString &path)>;

// 文档处理相关函数
DocumentPtr createFileDocument(const QString &file)
{
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
    } catch (const std::exception &e) {
        fmWarning() << "Check update failed:" << file << e.what();
        return false;
    }
}

bool isSupportedFile(const QString &path)
{
    QFileInfo fileInfo(path);
    if (!fileInfo.exists() || !fileInfo.isFile())
        return false;

    const QString &suffix = fileInfo.suffix().toLower();
    return DFMSEARCH::Global::isSupportedContentSearchExtension(suffix);
}

void processFile(const QString &path, const IndexWriterPtr &writer, ProgressReporter *reporter)
{
    try {
        if (!isSupportedFile(path))
            return;
#ifdef QT_DEBUG
        fmDebug() << "Adding [" << path << "]";
#endif
        writer->addDocument(createFileDocument(path));
        if (reporter) {
            reporter->increment();
        }
    } catch (const std::exception &e) {
        fmWarning() << "Process file failed:" << path << e.what();
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
            if (needAdd) {
#ifdef QT_DEBUG
                fmDebug() << "Adding [" << path << "]";
#endif
                writer->addDocument(createFileDocument(path));
            } else {
                fmDebug() << "Updating file [" << path << "]";
                TermPtr term = newLucene<Term>(L"path", path.toStdWString());
                writer->updateDocument(term, createFileDocument(path));
            }
        }

        if (reporter) {
            reporter->increment();
        }
    } catch (const std::exception &e) {
        fmWarning() << "Update file failed:" << path << e.what();
    }
}

}   // namespace

// 创建文件提供者
std::unique_ptr<FileProvider> TaskHandlers::createFileProvider(const QString &path)
{
    if (IndexUtility::isIndexWithAnything(path)) {
        QObject holder;
        SearchEngine *engine = SearchFactory::createEngine(SearchType::FileName, &holder);
        SearchOptions options;
        options.setSearchPath(QDir::rootPath());
        options.setSearchMethod(SearchMethod::Indexed);
        options.setIncludeHidden(true);
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
        fmWarning() << "Failed to get file list via ANYTHING!" << result.error().message();
    }
    return std::make_unique<FileSystemProvider>(path);
}

// 公开的任务处理函数实现
TaskHandler TaskHandlers::CreateIndexHandler()
{
    return [](const QString &path, TaskState &running) -> HandlerResult {
        fmInfo() << "Creating index for path:" << path;

        HandlerResult result { false, false };
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

            ProgressReporter reporter;
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
        HandlerResult result { false, false };

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

            // 使用文件提供者遍历文件
            auto provider = createFileProvider(path);
            if (!provider) {
                fmWarning() << "Failed to create file provider for path:" << path;
                return result;
            }

            ProgressReporter reporter;
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

TaskHandler TaskHandlers::RemoveIndexHandler()
{
    return [](const QString &pathList, TaskState &running) -> HandlerResult {
        fmInfo() << "Removing index for paths:" << pathList;
        HandlerResult result { false, false };

        try {
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

            // 将路径列表字符串转换为QStringList
            QStringList paths = pathList.split("|", Qt::SkipEmptyParts);

            ProgressReporter reporter;
            for (const QString &path : paths) {
                if (!running.isRunning())
                    break;

                try {
                    fmDebug() << "Removing index for path:" << path;
                    TermPtr term = newLucene<Term>(L"path", path.toStdWString());
                    writer->deleteDocuments(term);
                    reporter.increment();
                } catch (const std::exception &e) {
                    fmWarning() << "Failed to remove index for path:" << path << e.what();
                    // 继续处理其他路径
                }
            }

            if (!running.isRunning()) {
                fmWarning() << "Remove index task was interrupted";
                result.interrupted = true;
            }

            writer->optimize();
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
