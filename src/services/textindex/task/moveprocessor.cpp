// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "moveprocessor.h"
#include "utils/docutils.h"
#include "utils/indexutility.h"

#include <QFileInfo>

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
            fmDebug() << "File not found in index, skipping:" << fromPath;
            return true; // Not an error, file might not be indexed
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
            return true; // Not an error, directory might be empty or not indexed
        }

        fmInfo() << "Found" << allDocs->totalHits << "documents to update for directory move:" << fromPath;

        // Batch update all matching documents
        for (int32_t i = 0; i < allDocs->totalHits; ++i) {
            if (!running.isRunning()) {
                return false; // Interrupted
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
            return true; // No change needed
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