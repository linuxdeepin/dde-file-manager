// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "moveprocessor.h"
#include "utils/docutils.h"
#include "utils/indexutility.h"

#include <QFileInfo>
#include <QDateTime>

SERVICETEXTINDEX_USE_NAMESPACE
using namespace Lucene;

// FileMoveProcessor implementation
FileMoveProcessor::FileMoveProcessor(const SearcherPtr &searcher, const IndexWriterPtr &writer)
    : m_searcher(searcher), m_writer(writer)
{
}

bool FileMoveProcessor::processFileMove(const QString &fromPath, const QString &toPath)
{
    try {
        fmDebug() << "Processing file move:" << fromPath << "->" << toPath;

        TermQueryPtr pathQuery = newLucene<TermQuery>(
                newLucene<Term>(L"path", fromPath.toStdWString()));

        TopDocsPtr searchResult = m_searcher->search(pathQuery, 1);
        if (!searchResult || searchResult->totalHits == 0) {
            fmDebug() << "Source file not found in index:" << fromPath;

            // Smart detection: Check if this is an editor save pattern
            // (temporary file renamed to target file that should be indexed)
            if (IndexUtility::isSupportedFile(toPath) && isFileInIndex(toPath)) {
                fmInfo() << "Detected editor save pattern: temporary file" << fromPath
                         << "renamed to indexed file" << toPath << "- updating content";
                return processContentUpdate(toPath);
            }

            fmDebug() << "Target file not in index or not supported, skipping move operation";
            return true;   // Not an error, file might not be indexed
        }

        DocumentPtr doc = m_searcher->doc(searchResult->scoreDocs[0]->doc);
        if (!doc) {
            fmWarning() << "Failed to retrieve document for:" << fromPath;
            return false;
        }

        // Create new document with updated path
        DocumentPtr newDoc = DocUtils::copyFieldsExcept(doc, L"path");
        if (!newDoc) {
            fmWarning() << "Failed to copy document fields for:" << fromPath;
            return false;
        }

        // Add new path field
        newDoc->add(newLucene<Field>(L"path", toPath.toStdWString(),
                                     Field::STORE_YES, Field::INDEX_NOT_ANALYZED));

        // Update document in index
        TermPtr oldTerm = newLucene<Term>(L"path", fromPath.toStdWString());
        m_writer->updateDocument(oldTerm, newDoc);

        fmDebug() << "Updated file document path:" << fromPath << "->" << toPath;
        return true;
    } catch (const LuceneException &e) {
        fmWarning() << "File move processing failed with Lucene exception:" << fromPath
                    << QString::fromStdWString(e.getError());
        return false;
    } catch (const std::exception &e) {
        fmWarning() << "File move processing failed:" << fromPath << e.what();
        return false;
    }
}

bool FileMoveProcessor::isFileInIndex(const QString &path)
{
    try {
        TermQueryPtr pathQuery = newLucene<TermQuery>(
                newLucene<Term>(L"path", path.toStdWString()));

        TopDocsPtr searchResult = m_searcher->search(pathQuery, 1);
        return searchResult && searchResult->totalHits > 0;
    } catch (const LuceneException &e) {
        fmWarning() << "Failed to check if file exists in index:" << path
                    << QString::fromStdWString(e.getError());
        return false;
    } catch (const std::exception &e) {
        fmWarning() << "Failed to check if file exists in index:" << path << e.what();
        return false;
    }
}

bool FileMoveProcessor::processContentUpdate(const QString &filePath)
{
    try {
        fmInfo() << "Processing content update for file:" << filePath;

        // Create updated document with new content (similar to createFileDocument in taskhandler.cpp)
        DocumentPtr newDoc = newLucene<Document>();

        // file path
        newDoc->add(newLucene<Field>(L"path", filePath.toStdWString(),
                                     Field::STORE_YES, Field::INDEX_NOT_ANALYZED));

        // file last modified time
        QFileInfo fileInfo(filePath);
        const QDateTime modifyTime = fileInfo.lastModified();
        const QString modifyEpoch = QString::number(modifyTime.toSecsSinceEpoch());
        newDoc->add(newLucene<Field>(L"modified", modifyEpoch.toStdWString(),
                                     Field::STORE_YES, Field::INDEX_NOT_ANALYZED));

        // file name
        newDoc->add(newLucene<Field>(L"filename", fileInfo.fileName().toStdWString(),
                                     Field::STORE_YES, Field::INDEX_ANALYZED));

        // hidden tag
        QString hiddenTag = "N";
        if (DFMSEARCH::Global::isHiddenPathOrInHiddenDir(fileInfo.absoluteFilePath()))
            hiddenTag = "Y";
        newDoc->add(newLucene<Field>(L"is_hidden", hiddenTag.toStdWString(),
                                     Field::STORE_YES, Field::INDEX_NOT_ANALYZED));

        // file contents
        const auto &contentOpt = DocUtils::extractFileContent(filePath);
        if (contentOpt) {
            const QString &contents = contentOpt.value().trimmed();
            newDoc->add(newLucene<Field>(L"contents", contents.toStdWString(),
                                         Field::STORE_YES, Field::INDEX_ANALYZED));
        } else {
            fmWarning() << "Failed to extract content from file:" << filePath;
        }

        // Update the document in index
        TermPtr pathTerm = newLucene<Term>(L"path", filePath.toStdWString());
        m_writer->updateDocument(pathTerm, newDoc);

        fmInfo() << "Successfully updated file content in index:" << filePath;
        return true;
    } catch (const LuceneException &e) {
        fmWarning() << "Content update failed with Lucene exception:" << filePath
                    << QString::fromStdWString(e.getError());
        return false;
    } catch (const std::exception &e) {
        fmWarning() << "Content update failed:" << filePath << e.what();
        return false;
    } catch (...) {
        fmWarning() << "Content update failed with unknown exception:" << filePath;
        return false;
    }
}

// DirectoryMoveProcessor implementation
DirectoryMoveProcessor::DirectoryMoveProcessor(const SearcherPtr &searcher,
                                               const IndexWriterPtr &writer,
                                               const IndexReaderPtr &reader)
    : m_searcher(searcher), m_writer(writer), m_reader(reader)
{
}

bool DirectoryMoveProcessor::processDirectoryMove(const QString &fromPath, const QString &toPath, TaskState &running)
{
    try {
        fmDebug() << "Processing directory move:" << fromPath << "->" << toPath;

        QString normalizedFromPath = PathCalculator::normalizeDirectoryPath(fromPath);

        // Create prefix query to find all documents under this directory
        PrefixQueryPtr prefixQuery = newLucene<PrefixQuery>(
                newLucene<Term>(L"path", normalizedFromPath.toStdWString()));

        TopDocsPtr allDocs = m_searcher->search(prefixQuery, m_reader->maxDoc());
        if (!allDocs || allDocs->totalHits == 0) {
            fmDebug() << "No documents found for directory move:" << fromPath;
            return true;   // Not an error, directory might be empty or not indexed
        }

        fmInfo() << "Found" << allDocs->totalHits << "documents to update for directory move:" << fromPath;

        // Batch update all matching documents
        for (int32_t i = 0; i < allDocs->totalHits; ++i) {
            if (!running.isRunning()) {
                return false;   // Interrupted
            }

            if (!allDocs->scoreDocs || !allDocs->scoreDocs[i]) {
                continue;
            }

            DocumentPtr doc = m_searcher->doc(allDocs->scoreDocs[i]->doc);
            if (!doc) {
                continue;
            }

            if (!updateSingleDocumentPath(doc, normalizedFromPath, toPath)) {
                fmWarning() << "Failed to update document in directory move";
                // Continue with other documents
            }
        }

        return true;
    } catch (const LuceneException &e) {
        fmWarning() << "Directory move processing failed with Lucene exception:" << fromPath
                    << QString::fromStdWString(e.getError());
        return false;
    } catch (const std::exception &e) {
        fmWarning() << "Directory move processing failed:" << fromPath << e.what();
        return false;
    }
}

bool DirectoryMoveProcessor::updateSingleDocumentPath(const DocumentPtr &doc,
                                                      const QString &normalizedFromPath,
                                                      const QString &toPath)
{
    try {
        String oldPathValue = doc->get(L"path");
        QString oldPath = QString::fromStdWString(oldPathValue);

        // Calculate new path
        QString newPath = PathCalculator::calculateNewPathForDirectoryMove(oldPath, normalizedFromPath, toPath);

        if (newPath == oldPath) {
            return true;   // No change needed
        }

        // Create new document with updated path
        DocumentPtr newDoc = DocUtils::copyFieldsExcept(doc, L"path");
        if (!newDoc) {
            return false;
        }

        // Add new path field
        newDoc->add(newLucene<Field>(L"path", newPath.toStdWString(),
                                     Field::STORE_YES, Field::INDEX_NOT_ANALYZED));

        // Update document in index
        TermPtr oldTerm = newLucene<Term>(L"path", oldPathValue);
        m_writer->updateDocument(oldTerm, newDoc);

        fmDebug() << "Updated document path:" << oldPath << "->" << newPath;
        return true;
    } catch (const std::exception &e) {
        fmWarning() << "Failed to update single document path:" << e.what();
        return false;
    }
}
