// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILENAMEUTILS_H
#define FILENAMEUTILS_H

#include "dfmplugin_fileoperations_global.h"

#include <dfm-base/interfaces/fileinfo.h>

#include <QString>
#include <QUrl>

DPFILEOPERATIONS_BEGIN_NAMESPACE

/*!
 * \brief Structure to hold parsed file name components
 */
struct FileNameComponents
{
    QString baseName;   // Base name without extension
    QString completeSuffix;   // File extension
    QString fileName;   // Complete file name

    FileNameComponents() = default;
    FileNameComponents(const QString &base, const QString &ext, const QString &full)
        : baseName(base), completeSuffix(ext), fileName(full) { }
};

/*!
 * \brief Namespace for file name parsing operations
 *
 * Follows Single Responsibility Principle - only responsible for parsing file names
 */
namespace FileNameParser {
/*!
 * \brief Parse file name components from FileInfo
 * \param fileInfo File information object
 * \return Parsed file name components
 */
FileNameComponents parseFileName(FileInfoPointer fileInfo);

/*!
 * \brief Handle special case for hidden files (starting with dot)
 * \param components File name components to fix
 */
void fixHiddenFileBaseName(FileNameComponents &components);
}

/*!
 * \brief Namespace for file existence checking operations
 *
 * Follows Single Responsibility Principle - only responsible for checking file existence
 */
namespace FileExistenceChecker {
/*!
 * \brief Check if file exists in target directory
 * \param targetDir Target directory info
 * \param fileName File name to check
 * \return True if file exists, false otherwise
 */
bool fileExists(FileInfoPointer targetDir, const QString &fileName);

/*!
 * \brief Validate target directory
 * \param targetDir Target directory info
 * \return True if directory is valid and exists, false otherwise
 */
bool isValidTargetDirectory(FileInfoPointer targetDir);
}

/*!
 * \brief Namespace for conflict-free file name generation
 *
 * Follows Single Responsibility Principle - only responsible for generating unique names
 */
namespace ConflictNameGenerator {
/*!
 * \brief Generate a unique file name that doesn't conflict with existing files
 * \param components Original file name components
 * \param targetDir Target directory where file will be placed
 * \return Unique file name, or empty string if generation failed
 */
QString generateUniqueFileName(const FileNameComponents &components,
                               FileInfoPointer targetDir);

/*!
 * \brief Generate copy suffix for given number
 * \param number Copy number (0 for first copy, 1+ for numbered copies)
 * \return Formatted copy suffix
 */
QString generateCopySuffix(int number);

/*!
 * \brief Construct full file name from components and suffix
 * \param components File name components
 * \param copySuffix Copy suffix to append
 * \return Complete file name
 */
QString constructFileName(const FileNameComponents &components, const QString &copySuffix);
}

/*!
 * \brief Main namespace for file naming operations
 *
 * Follows Facade pattern - provides simple interface to complex subsystem
 * Follows Dependency Inversion Principle - depends on abstractions, not concrete classes
 */
namespace FileNamingUtils {
/*!
 * \brief Generate a non-conflicting file name for file operations
 * \param fromInfo Source file information
 * \param targetDir Target directory information
 * \return Non-conflicting file name, or empty string if generation failed
 */
QString generateNonConflictingName(FileInfoPointer fromInfo,
                                   FileInfoPointer targetDir);
}

DPFILEOPERATIONS_END_NAMESPACE

#endif   // FILENAMEUTILS_H
