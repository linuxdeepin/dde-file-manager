// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MOVEPROCESSOR_H
#define MOVEPROCESSOR_H

#include "service_textindex_global.h"
#include "utils/taskstate.h"

#include <lucene++/LuceneHeaders.h>
#include <QString>
#include <QSet>

SERVICETEXTINDEX_BEGIN_NAMESPACE

/**
 * @brief Processor for handling single file move operations in index
 */
class FileMoveProcessor
{
public:
    FileMoveProcessor(const Lucene::SearcherPtr &searcher, const Lucene::IndexWriterPtr &writer);

    /**
     * @brief Process single file move operation
     * @param fromPath Source file path
     * @param toPath Target file path
     * @return true if processed successfully, false otherwise
     */
    bool processFileMove(const QString &fromPath, const QString &toPath);

private:
    /**
     * @brief Check if a file exists in the index or has been processed in current batch
     * @param path File path to check
     * @return true if file exists in index or processed cache, false otherwise
     */
    bool isFileInIndex(const QString &path);



    /**
     * @brief Process content update for a file (re-index its content)
     * @param filePath Path of the file to re-index
     * @return true if content updated successfully, false otherwise
     */
    bool processContentUpdate(const QString &filePath);

    /**
     * @brief Process content update with cache management
     * @param filePath Path of the file to re-index
     * @param operation Description of the operation for logging
     * @return true if content updated successfully, false otherwise
     */
    bool processContentUpdateWithCache(const QString &filePath, const QString &operation);

    Lucene::SearcherPtr m_searcher;
    Lucene::IndexWriterPtr m_writer;
    QSet<QString> m_processedPaths; ///< Cache of paths processed in current batch (not yet committed)
};

/**
 * @brief Processor for handling directory move operations in index
 */
class DirectoryMoveProcessor
{
public:
    DirectoryMoveProcessor(const Lucene::SearcherPtr &searcher, 
                          const Lucene::IndexWriterPtr &writer, 
                          const Lucene::IndexReaderPtr &reader);

    /**
     * @brief Process directory move operation using prefix query
     * @param fromPath Source directory path
     * @param toPath Target directory path
     * @param running Task state for interruption checking
     * @return true if processed successfully, false if interrupted or failed
     */
    bool processDirectoryMove(const QString &fromPath, const QString &toPath, TaskState &running);

private:
    /**
     * @brief Update single document path within directory move
     * @param doc Document to update
     * @param normalizedFromPath Normalized source directory path
     * @param toPath Target directory path
     * @return true if updated successfully, false otherwise
     */
    bool updateSingleDocumentPath(const Lucene::DocumentPtr &doc, 
                                 const QString &normalizedFromPath, 
                                 const QString &toPath);

    Lucene::SearcherPtr m_searcher;
    Lucene::IndexWriterPtr m_writer;
    Lucene::IndexReaderPtr m_reader;
};

SERVICETEXTINDEX_END_NAMESPACE

#endif // MOVEPROCESSOR_H 