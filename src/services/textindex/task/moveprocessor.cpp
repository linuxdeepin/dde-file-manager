// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "moveprocessor.h"
#include "utils/docutils.h"
#include "utils/textindexconfig.h"

#include <dfm-search/field_names.h>

#include <QFileInfo>
#include <QDateTime>
#include <QLoggingCategory>

SERVICETEXTINDEX_USE_NAMESPACE
using namespace Lucene;
DFM_SEARCH_USE_NS
using namespace DFMSEARCH::LuceneFieldNames;

namespace {

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
                newLucene<Term>(pathField(m_context->profile()), fromPath.toStdWString()));

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

        DocumentPtr doc = m_searcher->doc(searchResult->scoreDocs[0]->doc);
        if (!doc) {
            fmWarning() << "[FileMoveProcessor::processFileMove] Failed to retrieve document for:" << fromPath;
            return false;
        }

        // Create new document with updated path and ancestor paths
        DocumentPtr newDoc = DocUtils::copyFieldsExcept(doc, { pathField(m_context->profile()), ancestorPathsField(m_context->profile()) });
        if (!newDoc) {
            fmWarning() << "[FileMoveProcessor::processFileMove] Failed to copy document fields for:" << fromPath;
            return false;
        }

        // Add new path field
        newDoc->add(newLucene<Field>(pathField(m_context->profile()), toPath.toStdWString(),
                                     Field::STORE_YES, Field::INDEX_NOT_ANALYZED));

        // Add new ancestor paths
        const QStringList ancestorPaths = PathCalculator::extractAncestorPaths(toPath);
        for (const QString &ancestorPath : ancestorPaths) {
            newDoc->add(newLucene<Field>(ancestorPathsField(m_context->profile()), ancestorPath.toStdWString(),
                                         Field::STORE_NO, Field::INDEX_NOT_ANALYZED));
        }

        // Update document in index
        TermPtr oldTerm = newLucene<Term>(pathField(m_context->profile()), fromPath.toStdWString());
        m_writer->updateDocument(oldTerm, newDoc);

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
                newLucene<Term>(pathField(m_context->profile()), path.toStdWString()));

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

        const TextIndexConfig &config = TextIndexConfig::instance();
        const int truncationSizeMB = m_context->profile().type() == IndexProfile::Type::Ocr
                ? config.maxOcrImageSizeMB()
                : config.maxIndexFileTruncationSizeMB();
        const size_t maxBytes = static_cast<size_t>(truncationSizeMB) * 1024 * 1024;
        const IndexExtractionResult extraction = m_context->extractor()->extract(filePath, maxBytes);
        if (!extraction.success) {
            fmWarning() << "[FileMoveProcessor::processContentUpdate] Failed to extract content from file:"
                        << filePath << "error:" << extraction.error;
        }

        DocumentPtr newDoc = m_context->documentBuilder()->build(filePath, extraction.text);

        // Update the document in index
        TermPtr pathTerm = newLucene<Term>(pathField(m_context->profile()), filePath.toStdWString());
        m_writer->updateDocument(pathTerm, newDoc);

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
                newLucene<Term>(ancestorPathsField(m_context->profile()), fromPath.toStdWString()));

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
        String oldPathValue = doc->get(pathField(m_context->profile()));
        QString oldPath = QString::fromStdWString(oldPathValue);

        // Calculate new path
        QString newPath = PathCalculator::calculateNewPathForDirectoryMove(oldPath, normalizedFromPath, toPath);

        if (newPath == oldPath) {
            fmDebug() << "[DirectoryMoveProcessor::updateSingleDocumentPath] No path change needed for:" << oldPath;
            return true;   // No change needed
        }

        // Create new document with updated path and ancestor paths
        DocumentPtr newDoc = DocUtils::copyFieldsExcept(doc, { pathField(m_context->profile()), ancestorPathsField(m_context->profile()) });
        if (!newDoc) {
            fmWarning() << "[DirectoryMoveProcessor::updateSingleDocumentPath] Failed to copy document fields for:" << oldPath;
            return false;
        }

        // Add new path field
        newDoc->add(newLucene<Field>(pathField(m_context->profile()), newPath.toStdWString(),
                                     Field::STORE_YES, Field::INDEX_NOT_ANALYZED));

        // Add new ancestor paths
        const QStringList ancestorPaths = PathCalculator::extractAncestorPaths(newPath);
        for (const QString &ancestorPath : ancestorPaths) {
            newDoc->add(newLucene<Field>(ancestorPathsField(m_context->profile()), ancestorPath.toStdWString(),
                                         Field::STORE_NO, Field::INDEX_NOT_ANALYZED));
        }

        // Update document in index
        TermPtr oldTerm = newLucene<Term>(pathField(m_context->profile()), oldPathValue);
        m_writer->updateDocument(oldTerm, newDoc);

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
