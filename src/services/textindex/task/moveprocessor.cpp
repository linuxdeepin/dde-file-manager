// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "moveprocessor.h"
#include "utils/docutils.h"

#include <QFileInfo>
#include <QDateTime>
#include <QLoggingCategory>

#include <list>

SERVICETEXTINDEX_USE_NAMESPACE
using namespace Lucene;
DFM_SEARCH_USE_NS

namespace {

/// 收集 builder 声明的路径派生字段重算规格；builder 缺省（如部分单测场景）时返回空列表
std::list<PathDerivedFieldSpec> collectDerivedSpecs(const IndexContext &context)
{
    const IndexDocumentBuilder *builder = context.documentBuilder();
    if (!builder)
        return {};
    return builder->pathDerivedFields();
}

/// 从旧文档复制字段（排除 path/ancestor_paths/路径派生字段），再按新路径补齐：
/// path 与 ancestor_paths 全量重算；派生字段用声明式重算器更新（纯元数据，零 IO）
DocumentPtr createMovedDocument(const IndexContext &context, const DocumentPtr &oldDoc,
                                const QString &toPath, const std::list<PathDerivedFieldSpec> &derivedSpecs)
{
    std::vector<Lucene::String> exclude = { context.profile().pathField(), context.profile().ancestorPathsField() };
    for (const auto &spec : derivedSpecs)
        exclude.push_back(spec.fieldName);

    DocumentPtr newDoc = DocUtils::copyFieldsExcept(oldDoc, exclude);
    if (!newDoc)
        return nullptr;

    newDoc->add(newLucene<Field>(context.profile().pathField(), toPath.toStdWString(),
                                 Field::STORE_YES, Field::INDEX_NOT_ANALYZED));

    for (const QString &ancestorPath : PathCalculator::extractAncestorPaths(toPath)) {
        newDoc->add(newLucene<Field>(context.profile().ancestorPathsField(), ancestorPath.toStdWString(),
                                     Field::STORE_NO, Field::INDEX_NOT_ANALYZED));
    }

    for (const auto &spec : derivedSpecs) {
        if (FieldPtr field = spec.rebuild(toPath))
            newDoc->add(field);
    }

    return newDoc;
}

}   // namespace

// FileMoveProcessor implementation
FileMoveProcessor::FileMoveProcessor(const IndexContext &context, const SearcherPtr &searcher, const IndexWriterPtr &writer)
    : m_searcher(searcher), m_writer(writer), m_context(&context)
{
    fmDebug() << "[FileMoveProcessor] Initialized with searcher and writer";
}

bool FileMoveProcessor::processFileMove(const QString &fromPath, const QString &toPath)
{
    try {
        fmInfo() << "[FileMoveProcessor::processFileMove] Processing file move:" << fromPath << "->" << toPath;

        TermQueryPtr pathQuery = newLucene<TermQuery>(
                newLucene<Term>(m_context->profile().pathField(), fromPath.toStdWString()));

        TopDocsPtr searchResult = m_searcher->search(pathQuery, 1);
        if (!searchResult || searchResult->totalHits == 0) {
            fmDebug() << "[FileMoveProcessor::processFileMove] Source file not found in index:" << fromPath;

            // Check if target file should be indexed
            if (m_context->profile().isCandidateFile(toPath) && QFileInfo(toPath).exists()) {
                if (isFileInIndex(toPath)) {
                    // Smart detection: Editor save pattern (temporary file renamed to indexed file)
                    fmInfo() << "[FileMoveProcessor::processFileMove] Detected editor save pattern - temporary file"
                             << fromPath << "renamed to indexed file" << toPath << "- updating content";
                    return processContentUpdateWithCache(toPath, "editor save pattern");
                } else {
                    // Fallback: Create new index entry for supported file not in index
                    // This handles cases like:
                    // 1. Files moved from unindexed directories
                    // 2. Index corruption or incomplete indexing
                    // 3. External file operations
                    fmInfo() << "[FileMoveProcessor::processFileMove] Fallback indexing for supported file not in index:"
                             << toPath;
                    return processContentUpdateWithCache(toPath, "fallback indexing");
                }
            }

            fmDebug() << "[FileMoveProcessor::processFileMove] Target file not in index or not supported, skipping move operation";
            return true;   // Not an error, file might not be indexed
        }

        // 按 profile 声明的 move 更新策略分发（策略语义见 IndexProfile::MoveUpdatePolicy）：
        // 文档字段由路径派生的 profile 必须整体重建，否则旧路径派生的字段值
        // （如 filename 的 file_name/pinyin）残留导致新名称搜索不到
        switch (m_context->profile().moveUpdatePolicy()) {
        case IndexProfile::MoveUpdatePolicy::RebuildDocument:
            return rebuildDocumentForMove(fromPath, toPath);
        case IndexProfile::MoveUpdatePolicy::UpdatePathOnly:
            break;
        }

        DocumentPtr doc = m_searcher->doc(searchResult->scoreDocs[0]->doc);
        if (!doc) {
            fmWarning() << "[FileMoveProcessor::processFileMove] Failed to retrieve document for:" << fromPath;
            return false;
        }

        // copy 保留与路径无关的字段（contents/checksum/时间戳等），按新路径
        // 重算 path/ancestor_paths 及 builder 声明的路径派生字段
        DocumentPtr newDoc = createMovedDocument(*m_context, doc, toPath, collectDerivedSpecs(*m_context));
        if (!newDoc) {
            fmWarning() << "[FileMoveProcessor::processFileMove] Failed to copy document fields for:" << fromPath;
            return false;
        }

        // Update document in index
        TermPtr oldTerm = newLucene<Term>(m_context->profile().pathField(), fromPath.toStdWString());
        m_writer->updateDocument(oldTerm, newDoc);
        m_hasChanges = true;

        // Update processed paths cache
        m_processedPaths.remove(fromPath);   // Remove old path
        m_processedPaths.insert(toPath);   // Add new path

        fmInfo() << "[FileMoveProcessor::processFileMove] Successfully updated file document path:"
                 << fromPath << "->" << toPath;
        return true;
    } catch (const LuceneException &e) {
        fmWarning() << "[FileMoveProcessor::processFileMove] File move processing failed with Lucene exception:"
                    << fromPath << "error:" << QString::fromStdWString(e.getError());
        return false;
    } catch (const std::exception &e) {
        fmWarning() << "[FileMoveProcessor::processFileMove] File move processing failed with exception:"
                    << fromPath << "error:" << e.what();
        return false;
    }
}

bool FileMoveProcessor::rebuildDocumentForMove(const QString &fromPath, const QString &toPath)
{
    if (!m_context->documentBuilder()) {
        fmWarning() << "[FileMoveProcessor::rebuildDocumentForMove] Missing document builder for profile:"
                    << m_context->profile().id();
        return false;
    }

    try {
        DocumentPtr newDoc = m_context->documentBuilder()->build(toPath, QString());
        if (!newDoc) {
            fmWarning() << "[FileMoveProcessor::rebuildDocumentForMove] Failed to rebuild document for:" << toPath;
            return false;
        }

        // 以 fromPath 定位更新，避免旧路径文档残留
        TermPtr oldTerm = newLucene<Term>(m_context->profile().pathField(), fromPath.toStdWString());
        m_writer->updateDocument(oldTerm, newDoc);
        m_hasChanges = true;

        m_processedPaths.remove(fromPath);
        m_processedPaths.insert(toPath);

        fmInfo() << "[FileMoveProcessor::rebuildDocumentForMove] Rebuilt document for move:"
                 << fromPath << "->" << toPath;
        return true;
    } catch (const LuceneException &e) {
        fmWarning() << "[FileMoveProcessor::rebuildDocumentForMove] Failed with Lucene exception:"
                    << fromPath << "->" << toPath
                    << "error:" << QString::fromStdWString(e.getError());
        return false;
    } catch (const std::exception &e) {
        fmWarning() << "[FileMoveProcessor::rebuildDocumentForMove] Failed with exception:"
                    << fromPath << "->" << toPath << "error:" << e.what();
        return false;
    } catch (...) {
        fmWarning() << "[FileMoveProcessor::rebuildDocumentForMove] Failed with unknown exception:"
                    << fromPath << "->" << toPath;
        return false;
    }
}

bool FileMoveProcessor::isFileInIndex(const QString &path)
{
    try {
        // First check if the file has been processed in current batch (but not yet committed)
        if (m_processedPaths.contains(path)) {
            fmDebug() << "[FileMoveProcessor::isFileInIndex] File found in processed cache:" << path;
            return true;
        }

        // Then check in the actual index
        TermQueryPtr pathQuery = newLucene<TermQuery>(
                newLucene<Term>(m_context->profile().pathField(), path.toStdWString()));

        TopDocsPtr searchResult = m_searcher->search(pathQuery, 1);
        bool exists = searchResult && searchResult->totalHits > 0;

        fmDebug() << "[FileMoveProcessor::isFileInIndex] File existence check:" << path << "exists:" << exists;
        return exists;
    } catch (const LuceneException &e) {
        fmWarning() << "[FileMoveProcessor::isFileInIndex] Failed to check file existence with Lucene exception:"
                    << path << "error:" << QString::fromStdWString(e.getError());
        return false;
    } catch (const std::exception &e) {
        fmWarning() << "[FileMoveProcessor::isFileInIndex] Failed to check file existence with exception:"
                    << path << "error:" << e.what();
        return false;
    }
}

bool FileMoveProcessor::processContentUpdate(const QString &filePath)
{
    try {
        fmInfo() << "[FileMoveProcessor::processContentUpdate] Processing content update for file:" << filePath;
        QFileInfo fileInfo(filePath);
        if (!fileInfo.exists()) {
            fmWarning() << "[FileMoveProcessor::processContentUpdate] File does not exist:" << filePath;
            return false;
        }

        // filename profile 无内容提取（extractor() == nullptr）：直接用空文本构建文档，
        // 与 createFileDocument 的处理一致（FileNameDocumentBuilder 忽略 text 参数）。
        // 此处必须判空：MoveFileList 的 editor-save/fallback 路径在 filename profile 下
        // 也会进入本函数，对 nullptr 调用 extract() 会直接 SIGSEGV。
        QString text;
        if (m_context->extractor()) {
            const int truncationSizeMB = m_context->profile().maxFileTruncationSizeMB();
            const size_t maxBytes = static_cast<size_t>(truncationSizeMB) * 1024 * 1024;
            const IndexExtractionResult extraction = m_context->extractor()->extract(filePath, maxBytes);
            if (!extraction.success) {
                fmInfo() << "[FileMoveProcessor::processContentUpdate] Failed to extract content from file:"
                         << filePath << "error:" << extraction.error;
            }
            text = extraction.text;
        }

        DocumentPtr newDoc = m_context->documentBuilder()->build(filePath, text);

        // Update the document in index
        TermPtr pathTerm = newLucene<Term>(m_context->profile().pathField(), filePath.toStdWString());
        m_writer->updateDocument(pathTerm, newDoc);
        m_hasChanges = true;

        fmInfo() << "[FileMoveProcessor::processContentUpdate] Successfully updated file content in index:" << filePath;
        return true;
    } catch (const LuceneException &e) {
        fmWarning() << "[FileMoveProcessor::processContentUpdate] Content update failed with Lucene exception:"
                    << filePath << "error:" << QString::fromStdWString(e.getError());
        return false;
    } catch (const std::exception &e) {
        fmWarning() << "[FileMoveProcessor::processContentUpdate] Content update failed with exception:"
                    << filePath << "error:" << e.what();
        return false;
    } catch (...) {
        fmWarning() << "[FileMoveProcessor::processContentUpdate] Content update failed with unknown exception:" << filePath;
        return false;
    }
}

bool FileMoveProcessor::processContentUpdateWithCache(const QString &filePath, const QString &operation)
{
    bool success = processContentUpdate(filePath);
    if (success) {
        // Update processed paths cache
        m_processedPaths.insert(filePath);
        fmInfo() << "[FileMoveProcessor::processContentUpdateWithCache] Successfully completed" << operation << "for:" << filePath;
    } else {
        fmWarning() << "[FileMoveProcessor::processContentUpdateWithCache] Failed" << operation << "for:" << filePath;
    }
    return success;
}

// DirectoryMoveProcessor implementation
DirectoryMoveProcessor::DirectoryMoveProcessor(const IndexContext &context,
                                               const SearcherPtr &searcher,
                                               const IndexWriterPtr &writer,
                                               const IndexReaderPtr &reader)
    : m_searcher(searcher), m_writer(writer), m_reader(reader), m_context(&context)
{
    fmDebug() << "[DirectoryMoveProcessor] Initialized with searcher, writer, and reader";
}

bool DirectoryMoveProcessor::processDirectoryMove(const QString &fromPath, const QString &toPath, TaskState &running)
{
    try {
        fmInfo() << "[DirectoryMoveProcessor::processDirectoryMove] Processing directory move:"
                 << fromPath << "->" << toPath;

        // 使用 TermQuery 在 ancestor_paths 字段上进行精确匹配
        // ancestor_paths 存储的目录路径不带尾部斜杠
        TermQueryPtr ancestorQuery = newLucene<TermQuery>(
                newLucene<Term>(m_context->profile().ancestorPathsField(), fromPath.toStdWString()));

        TopDocsPtr allDocs = m_searcher->search(ancestorQuery, m_reader->maxDoc());
        if (!allDocs || allDocs->totalHits == 0) {
            fmDebug() << "[DirectoryMoveProcessor::processDirectoryMove] No documents found for directory move:" << fromPath;
            return true;   // Not an error, directory might be empty or not indexed
        }

        fmInfo() << "[DirectoryMoveProcessor::processDirectoryMove] Found" << allDocs->totalHits
                 << "documents to update for directory move:" << fromPath;

        int successCount = 0;
        int failureCount = 0;

        // 用于计算新路径时使用，需要带尾部斜杠
        QString normalizedFromPath = PathCalculator::normalizeDirectoryPath(fromPath);

        // Batch update all matching documents
        for (int32_t i = 0; i < allDocs->totalHits; ++i) {
            if (!running.isRunning()) {
                fmInfo() << "[DirectoryMoveProcessor::processDirectoryMove] Directory move interrupted by user request";
                return false;   // Interrupted
            }

            if (!allDocs->scoreDocs || !allDocs->scoreDocs[i]) {
                fmWarning() << "[DirectoryMoveProcessor::processDirectoryMove] Null scoreDoc at index:" << i;
                failureCount++;
                continue;
            }

            DocumentPtr doc = m_searcher->doc(allDocs->scoreDocs[i]->doc);
            if (!doc) {
                fmWarning() << "[DirectoryMoveProcessor::processDirectoryMove] Null document at index:" << i;
                failureCount++;
                continue;
            }

            if (updateSingleDocumentPath(doc, normalizedFromPath, toPath)) {
                successCount++;
            } else {
                fmWarning() << "[DirectoryMoveProcessor::processDirectoryMove] Failed to update document at index:" << i;
                failureCount++;
                // Continue with other documents
            }
        }

        fmInfo() << "[DirectoryMoveProcessor::processDirectoryMove] Directory move completed - successful updates:"
                 << successCount << "failed updates:" << failureCount;
        return true;
    } catch (const LuceneException &e) {
        fmWarning() << "[DirectoryMoveProcessor::processDirectoryMove] Directory move processing failed with Lucene exception:"
                    << fromPath << "error:" << QString::fromStdWString(e.getError());
        return false;
    } catch (const std::exception &e) {
        fmWarning() << "[DirectoryMoveProcessor::processDirectoryMove] Directory move processing failed with exception:"
                    << fromPath << "error:" << e.what();
        return false;
    }
}

bool DirectoryMoveProcessor::updateSingleDocumentPath(const DocumentPtr &doc,
                                                      const QString &normalizedFromPath,
                                                      const QString &toPath)
{
    try {
        String oldPathValue = doc->get(m_context->profile().pathField());
        QString oldPath = QString::fromStdWString(oldPathValue);

        // Calculate new path
        QString newPath = PathCalculator::calculateNewPathForDirectoryMove(oldPath, normalizedFromPath, toPath);

        if (newPath == oldPath) {
            fmDebug() << "[DirectoryMoveProcessor::updateSingleDocumentPath] No path change needed for:" << oldPath;
            return true;   // No change needed
        }

        // 目录改名不改变子文件的 basename（file_name/pinyin/file_ext 等值不变），
        // 统一走 copy 保留 + 派生字段重算：path/ancestor_paths 全量重算，
        // is_hidden 等完整路径派生字段由 builder 声明的重算器更新。
        // 即使 RebuildDocument 策略的 profile（filename）也不整体重建，
        // 避免百万级子文件的目录改名时逐文档 stat/重建的开销
        DocumentPtr newDoc = createMovedDocument(*m_context, doc, newPath, collectDerivedSpecs(*m_context));

        if (!newDoc) {
            fmWarning() << "[DirectoryMoveProcessor::updateSingleDocumentPath] Failed to build document for:" << newPath;
            return false;
        }

        // Update document in index
        TermPtr oldTerm = newLucene<Term>(m_context->profile().pathField(), oldPathValue);
        m_writer->updateDocument(oldTerm, newDoc);
        m_hasChanges = true;

        fmDebug() << "[DirectoryMoveProcessor::updateSingleDocumentPath] Successfully updated document path:"
                  << oldPath << "->" << newPath;
        return true;
    } catch (const LuceneException &e) {
        fmWarning() << "[DirectoryMoveProcessor::updateSingleDocumentPath] Failed to update document path with Lucene exception:"
                    << "error:" << QString::fromStdWString(e.getError());
        return false;
    } catch (const std::exception &e) {
        fmWarning() << "[DirectoryMoveProcessor::updateSingleDocumentPath] Failed to update document path with exception:"
                    << "error:" << e.what();
        return false;
    }
}
