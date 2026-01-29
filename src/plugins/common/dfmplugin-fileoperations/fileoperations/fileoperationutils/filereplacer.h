// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEREPLACER_H
#define FILEREPLACER_H

#include "dfmplugin_fileoperations_global.h"
#include <QString>
#include <QUrl>

DPFILEOPERATIONS_BEGIN_NAMESPACE

/*!
 * \brief FileReplacementContext - Context for safe file replacement
 *
 * Stores information needed to perform atomic file replacement.
 * This is created at execution time when we know the actual target path
 * and whether the target already exists and needs to be replaced.
 */
class FileReplacementContext
{
public:
    /*!
     * \brief Original target path (what user expects as final destination)
     */
    QString originalTargetPath;

    /*!
     * \brief Temporary file path (where we actually write to)
     * Empty if not using temporary file strategy
     */
    QString temporaryFilePath;

    /*!
     * \brief Check if this context represents a replacement operation
     */
    bool isReplacement() const
    {
        return !originalTargetPath.isEmpty() && !temporaryFilePath.isEmpty();
    }

    /*!
     * \brief Reset context to empty state
     */
    void reset()
    {
        originalTargetPath.clear();
        temporaryFilePath.clear();
    }
};

/*!
 * \brief Namespace FileReplacer - Safe file replacement utilities
 *
 * Responsible for:
 * - Determining if temporary file strategy should be used for a given target
 * - Creating replacement context with temporary paths
 * - Performing atomic rename to finalize replacement
 */
namespace FileReplacer {

/*!
 * \brief Determine if temporary file strategy should be used
 *
 * Used when:
 * - Target is on local filesystem
 * - Target file already exists
 * - Target is not a symlink (symlinks are deleted directly)
 *
 * \param targetPath Path of target file that would be overwritten
 * \return true if temporary file should be used
 */
bool shouldUseTemporaryFile(const QString &targetPath);

/*!
 * \brief Create replacement context for safe file replacement
 *
 * Analyzes target and decides whether to use temporary file strategy.
 * If yes, generates temporary path and returns context.
 * If no, returns empty context (direct overwrite).
 *
 * \param targetPath Original target path (destination file)
 * \return Context with replacement info, or empty if direct overwrite
 */
FileReplacementContext createReplacementContext(const QString &targetPath);

/*!
 * \brief Generate temporary file path for safe replacement
 *
 * Format: /path/.ddefileop-xxxxxxxx
 * Uses hidden file + random suffix to ensure uniqueness and avoid filename length issues
 *
 * \param targetPath Original target path
 * \return Temporary file path
 */
QString generateTemporaryPath(const QString &targetPath);

/*!
 * \brief Perform atomic replacement of target with temporary file
 *
 * Operations (atomic at filesystem level):
 * 1. Unlink original target file
 * 2. Rename temporary file to target location
 *
 * On any failure, temporary file is cleaned up.
 *
 * \param context Replacement context (must be valid, i.e., isReplacement() == true)
 * \return true on success, false on failure
 */
bool applyReplacement(const FileReplacementContext &context);

}   // namespace FileReplacer

DPFILEOPERATIONS_END_NAMESPACE
#endif   // FILEREPLACER_H
