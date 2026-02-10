// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filenameutils.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-io/dfile.h>

#include <QObject>
#include <QRegularExpression>
#include <QDebug>
#include <QMimeDatabase>
#include <QDir>

DFMBASE_USE_NAMESPACE

DPFILEOPERATIONS_BEGIN_NAMESPACE

namespace FileNameParser {

/*!
 * \brief Parse file name components from FileInfo
 */
FileNameComponents parseFileName(FileInfoPointer fileInfo)
{
    if (!fileInfo) {
        fmWarning() << "FileNameParser: Invalid file info provided";
        return FileNameComponents();
    }

    QString fileName = fileInfo->nameOf(NameInfoType::kFileName);
    if (fileName.isEmpty()) {
        fmWarning() << "FileNameParser: Empty file name from file info";
        return FileNameComponents();
    }

    QString baseName;
    QString completeSuffix;

    // For directories, never treat them as having file extensions
    // This prevents issues like "2025.09.02" being parsed as "2025.09" + ".02"
    if (fileInfo->isAttributes(OptInfoType::kIsDir)) {
        baseName = fileName;
        completeSuffix = QString();
        fmDebug() << "FileNameParser: Directory detected, treating entire name as basename:" << fileName;
    } else {
        // For files, use QMimeDatabase to intelligently detect file extensions (inspired by KDE's approach)
        QMimeDatabase db;
        QString nameSuffix = db.suffixForFileName(fileName);

        if (fileName.lastIndexOf(QLatin1Char('.')) == 0) {
            // Hidden file like .bashrc or .config - treat whole name as basename
            baseName = fileName;
            completeSuffix = QString();
        } else if (fileName.endsWith(QLatin1Char('.'))) {
            // File ending with dot like "file." - basename is everything except the trailing dot
            baseName = fileName.left(fileName.length() - 1);
            completeSuffix = QString();   // Empty suffix, but we'll add the dot back when constructing
        } else if (nameSuffix.isEmpty()) {
            // No recognized MIME extension, check for manual dot
            const int lastDot = fileName.lastIndexOf(QLatin1Char('.'));
            if (lastDot == -1) {
                // No dot at all - whole name is basename
                baseName = fileName;
                completeSuffix = QString();
            } else {
                // Has dot but no recognized MIME extension - treat as simple extension
                baseName = fileName.left(lastDot);
                completeSuffix = fileName.mid(lastDot + 1);
            }
        } else {
            // Has recognized MIME extension - use it as the complete suffix
            const QString fullSuffix = QLatin1Char('.') + nameSuffix;
            baseName = fileName.left(fileName.length() - fullSuffix.length());
            completeSuffix = nameSuffix;
        }
    }

    FileNameComponents components(baseName, completeSuffix, fileName);

    return components;
}

}   // namespace FileNameParser

namespace FileExistenceChecker {

/*!
 * \brief Validate target directory
 */
bool isValidTargetDirectory(FileInfoPointer targetDir)
{
    if (!targetDir) {
        fmWarning() << "FileExistenceChecker: Invalid target directory provided";
        return false;
    }

    if (!DFMIO::DFile(targetDir->urlOf(UrlInfoType::kUrl)).exists()) {
        fmWarning() << "FileExistenceChecker: Target directory does not exist:" << targetDir->urlOf(UrlInfoType::kUrl);
        return false;
    }

    if (!targetDir->isAttributes(OptInfoType::kIsDir)) {
        fmWarning() << "FileExistenceChecker: Target is not a directory:" << targetDir->urlOf(UrlInfoType::kUrl);
        return false;
    }

    return true;
}

/*!
 * \brief Check if file exists in target directory
 */
bool fileExists(FileInfoPointer targetDir, const QString &fileName)
{
    if (!isValidTargetDirectory(targetDir) || fileName.isEmpty()) {
        return false;
    }

    QUrl targetUrl = targetDir->urlOf(UrlInfoType::kUrl);
    QString cleanPath = QDir::cleanPath(targetUrl.path() + QLatin1Char('/') + fileName);
    targetUrl.setPath(cleanPath);
    return DFMIO::DFile(targetUrl).exists();
}

}   // namespace FileExistenceChecker

namespace ConflictNameGenerator {

/*!
 * \brief Generate copy suffix for given number
 */
QString generateCopySuffix(int number)
{
    const QString copySuffix = QObject::tr(" (copy)", "this should be translated in Noun version rather Verb, the first space should be ignore if translate to Chinese");
    const QString copySuffix2 = QObject::tr(" (copy %1)", "this should be translated in Noun version rather Verb, the first space should be ignore if translate to Chinese");

    return number > 0 ? copySuffix2.arg(number) : copySuffix;
}

/*!
 * \brief Construct full file name from components and suffix
 */
QString constructFileName(const FileNameComponents &components, const QString &copySuffix)
{
    // Special case: file ending with dot like "file." -> "file.(copy)"
    if (components.fileName.endsWith(QLatin1Char('.')) && components.completeSuffix.isEmpty()) {
        return QString("%1.%2").arg(components.baseName, copySuffix.trimmed());
    }

    QString newFileName = QString("%1%2").arg(components.baseName, copySuffix);

    if (!components.completeSuffix.isEmpty()) {
        newFileName.append('.').append(components.completeSuffix);
    }

    return newFileName;
}

/*!
 * \brief Generate a unique file name that doesn't conflict with existing files
 */
QString generateUniqueFileName(const FileNameComponents &components,
                               FileInfoPointer targetDir)
{
    if (!FileExistenceChecker::isValidTargetDirectory(targetDir)) {
        return QString();
    }

    if (components.baseName.isEmpty()) {
        fmWarning() << "ConflictNameGenerator: Empty base name provided";
        return QString();
    }

    int number = 0;
    QString candidateName;

    do {
        const QString copySuffix = generateCopySuffix(number);
        candidateName = constructFileName(components, copySuffix);
        ++number;

        // Safety check to prevent infinite loops
        if (number == std::numeric_limits<int>::max()) {
            fmCritical() << "ConflictNameGenerator: Too many naming attempts, aborting";
            return QString();
        }

    } while (FileExistenceChecker::fileExists(targetDir, candidateName));

    return candidateName;
}

}   // namespace ConflictNameGenerator

namespace SymlinkNameGenerator {

/*!
 * \brief Generate symlink suffix for given number
 */
QString generateSymlinkSuffix(int number)
{
    const QString shortcut = QObject::tr("Shortcut");

    return number > 0 ? QString(" %1%2").arg(shortcut, QString::number(number))
                      : QString(" %1").arg(shortcut);
}

/*!
 * \brief Construct symlink file name from components and suffix
 */
QString constructSymlinkFileName(const FileNameComponents &components, const QString &symlinkSuffix)
{
    // Special case: file ending with dot like "file." -> "file. Shortcut"
    if (components.fileName.endsWith(QLatin1Char('.')) && components.completeSuffix.isEmpty()) {
        return QString("%1.%2").arg(components.baseName, symlinkSuffix.trimmed());
    }

    QString newFileName = QString("%1%2").arg(components.baseName, symlinkSuffix);

    if (!components.completeSuffix.isEmpty()) {
        newFileName.append('.').append(components.completeSuffix);
    }

    return newFileName;
}

/*!
 * \brief Generate a unique symlink name that doesn't conflict with existing files or symlinks
 */
QString generateUniqueSymlinkName(const FileNameComponents &components,
                                  FileInfoPointer targetDir)
{
    if (!FileExistenceChecker::isValidTargetDirectory(targetDir)) {
        return QString();
    }

    if (components.baseName.isEmpty()) {
        fmWarning() << "SymlinkNameGenerator: Empty base name provided";
        return QString();
    }

    int number = 0;
    QString candidateName;

    do {
        const QString symlinkSuffix = generateSymlinkSuffix(number);
        candidateName = constructSymlinkFileName(components, symlinkSuffix);
        ++number;

        // Safety check to prevent infinite loops
        if (number == std::numeric_limits<int>::max()) {
            fmCritical() << "SymlinkNameGenerator: Too many naming attempts, aborting";
            return QString();
        }

    } while (FileExistenceChecker::fileExists(targetDir, candidateName));

    return candidateName;
}

}   // namespace SymlinkNameGenerator

namespace FileNamingUtils {

/*!
 * \brief Generate a non-conflicting file name for file operations
 */
QString generateNonConflictingName(FileInfoPointer fromInfo, FileInfoPointer targetDir)
{
    if (!fromInfo) {
        fmWarning() << "FileNamingUtils: Invalid source file info provided";
        return QString();
    }

    // Parse the file name components
    FileNameComponents components = FileNameParser::parseFileName(fromInfo);

    if (components.baseName.isEmpty()) {
        fmWarning() << "FileNamingUtils: Failed to parse file name components";
        return QString();
    }

    // Generate unique file name
    return ConflictNameGenerator::generateUniqueFileName(components, targetDir);
}

/*!
 * \brief Generate a non-conflicting symlink name for symlink operations
 */
QString generateNonConflictingSymlinkName(FileInfoPointer fromInfo, FileInfoPointer targetDir)
{
    if (!fromInfo) {
        fmWarning() << "FileNamingUtils: Invalid source file info provided for symlink";
        return QString();
    }

    // Handle the special case: displayName != fileName (e.g., desktop files)
    QString baseName;
    QString displayName = fromInfo->displayOf(DisPlayInfoType::kFileDisplayName);
    QString fileName = fromInfo->nameOf(NameInfoType::kFileName);

    if (displayName == fileName) {
        // Standard case: use MIME-type based parsing for better extension handling
        FileNameComponents components = FileNameParser::parseFileName(fromInfo);
        if (components.baseName.isEmpty()) {
            fmWarning() << "FileNamingUtils: Failed to parse file name components for symlink";
            return QString();
        }
        return SymlinkNameGenerator::generateUniqueSymlinkName(components, targetDir);
    } else {
        // Special case: displayName != fileName (e.g., desktop files)
        // Use displayName as baseName directly without extension parsing
        FileNameComponents components;
        components.baseName = displayName;
        components.completeSuffix = QString();   // No extension for display names
        components.fileName = displayName;

        fmInfo() << "FileNamingUtils: Using display name for symlink:" << displayName << "vs fileName:" << fileName;
        return SymlinkNameGenerator::generateUniqueSymlinkName(components, targetDir);
    }
}

}   // namespace FileNamingUtils

DPFILEOPERATIONS_END_NAMESPACE
