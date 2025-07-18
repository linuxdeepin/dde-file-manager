// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mimetypedisplaymanager.h"

#include <dfm-base/base/standardpaths.h>

#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <algorithm>   // Required for std::sort
#include <QMimeDatabase>
#include <QMimeType>

using namespace dfmbase;

MimeTypeDisplayManager::MimeTypeDisplayManager(QObject *parent)
    : QObject(parent)
{
    initData();
}

MimeTypeDisplayManager::~MimeTypeDisplayManager()
{
}

void MimeTypeDisplayManager::initData()
{
    displayNamesMap[FileInfo::FileType::kDirectory] = tr("Directory");
    displayNamesMap[FileInfo::FileType::kDesktopApplication] = tr("Application");
    displayNamesMap[FileInfo::FileType::kVideos] = tr("Video");
    displayNamesMap[FileInfo::FileType::kAudios] = tr("Audio");
    displayNamesMap[FileInfo::FileType::kImages] = tr("Image");
    displayNamesMap[FileInfo::FileType::kArchives] = tr("Archive");
    displayNamesMap[FileInfo::FileType::kDocuments] = tr("Text");
    displayNamesMap[FileInfo::FileType::kExecutable] = tr("Executable");
    displayNamesMap[FileInfo::FileType::kBackups] = tr("Backup file");
    displayNamesMap[FileInfo::FileType::kUnknown] = tr("Unknown");

    namesMap[FileInfo::FileType::kDirectory] = "Directory";
    namesMap[FileInfo::FileType::kDesktopApplication] = "Application";
    namesMap[FileInfo::FileType::kVideos] = "Video";
    namesMap[FileInfo::FileType::kAudios] = "Audio";
    namesMap[FileInfo::FileType::kImages] = "Image";
    namesMap[FileInfo::FileType::kArchives] = "Archive";
    namesMap[FileInfo::FileType::kDocuments] = "Text";
    namesMap[FileInfo::FileType::kExecutable] = "Executable";
    namesMap[FileInfo::FileType::kBackups] = "Backup file";
    namesMap[FileInfo::FileType::kUnknown] = "Unknown";

    defaultIconNames[FileInfo::FileType::kDirectory] = "folder";
    defaultIconNames[FileInfo::FileType::kDesktopApplication] = "application-default-icon";
    defaultIconNames[FileInfo::FileType::kVideos] = "video";
    defaultIconNames[FileInfo::FileType::kAudios] = "music";
    defaultIconNames[FileInfo::FileType::kImages] = "image";
    defaultIconNames[FileInfo::FileType::kArchives] = "application-x-archive";
    defaultIconNames[FileInfo::FileType::kDocuments] = "text-plain";
    defaultIconNames[FileInfo::FileType::kExecutable] = "application-x-executable";
    defaultIconNames[FileInfo::FileType::kBackups] = "application-x-archive";   // generic backup file icon?
    defaultIconNames[FileInfo::FileType::kUnknown] = "application-default-icon";

    loadSupportMimeTypes();
}

QMimeType MimeTypeDisplayManager::fastDetermineMimeType(const QString &filePath) const
{
    if (filePath.isEmpty())
        return QMimeType();

    QMimeType mimeType = mimeTypeDatabase.mimeTypeForFile(filePath, QMimeDatabase::MatchExtension);
    if (mimeType.isValid() && mimeType.name() != "application/octet-stream")
        return mimeType;

    // Fallback: Check if it's a directory
    QFileInfo fileInfo(filePath);
    if (fileInfo.isDir()) {
        return mimeTypeDatabase.mimeTypeForName("inode/directory");
    }

    // Fallback to content-based detection limited by threshold
    if (contentBasedFallbackCount < kMaxContentBasedFallback) {
        ++contentBasedFallbackCount;
        qCWarning(logDFMBase) << "Fallback to content-based mime detection, attempt" << contentBasedFallbackCount << "for file:" << filePath;
        mimeType = mimeTypeDatabase.mimeTypeForFile(filePath, QMimeDatabase::MatchContent);
        if (mimeType.isValid())
            return mimeType;
    }

    return mimeType;   // May be invalid, but we skip further MatchContent
}

QMimeType MimeTypeDisplayManager::accurateLocalMimeType(const QString &filePath) const
{
    Q_ASSERT(!filePath.isEmpty());

    return mimeTypeDatabase.mimeTypeForFile(filePath);
}

QString MimeTypeDisplayManager::fastDisplayTypeFromPath(const QString &filePath) const
{
    QMimeType mimeType = fastDetermineMimeType(filePath);
    if (!mimeType.isValid())
        return displayNamesMap[FileInfo::FileType::kUnknown];

    return displayName(mimeType.name());
}

QString MimeTypeDisplayManager::fastMimeTypeName(const QString &filePath) const
{
    QMimeType mimeType = fastDetermineMimeType(filePath);
    if (!mimeType.isValid())
        return displayNamesMap[FileInfo::FileType::kUnknown];

    return fullMimeName(mimeType.name());
}

QString MimeTypeDisplayManager::accurateDisplayTypeFromPath(const QString &filePath) const
{
    QMimeType mimeType = accurateLocalMimeType(filePath);
    if (!mimeType.isValid())
        return displayNamesMap[FileInfo::FileType::kUnknown];

    return displayName(mimeType.name());
}

QString MimeTypeDisplayManager::accurateLocalMimeTypeName(const QString &filePath) const
{
    QMimeType mimeType = accurateLocalMimeType(filePath);
    if (!mimeType.isValid())
        return displayNamesMap[FileInfo::FileType::kUnknown];

    return fullMimeName(mimeType.name());
}

QString MimeTypeDisplayManager::displayName(const QString &mimeType) const
{
#ifdef QT_DEBUG
    return displayNamesMap.value(displayNameToEnum(mimeType)) + " (" + mimeType + ")";
#endif   // Q_DEBUG
    return displayNamesMap.value(displayNameToEnum(mimeType));
}

QString MimeTypeDisplayManager::fullMimeName(const QString &mimeType) const
{
    return namesMap.value(displayNameToEnum(mimeType)) + " (" + mimeType + ")";
}

FileInfo::FileType MimeTypeDisplayManager::displayNameToEnum(const QString &mimeType) const
{
    if (mimeType == "application/x-desktop") {
        return FileInfo::FileType::kDesktopApplication;
    } else if (mimeType == "inode/directory") {
        return FileInfo::FileType::kDirectory;
    } else if (mimeType == "application/x-executable" || executableMimeTypes.contains(mimeType)) {
        return FileInfo::FileType::kExecutable;
    } else if (mimeType.startsWith("video/") || videoMimeTypes.contains(mimeType)) {
        return FileInfo::FileType::kVideos;
    } else if (mimeType.startsWith("audio/") || audioMimeTypes.contains(mimeType)) {
        return FileInfo::FileType::kAudios;
    } else if (mimeType.startsWith("image/") || imageMimeTypes.contains(mimeType)) {
        return FileInfo::FileType::kImages;
    } else if (mimeType.startsWith("text/") || textMimeTypes.contains(mimeType)) {
        return FileInfo::FileType::kDocuments;
    } else if (archiveMimeTypes.contains(mimeType)) {
        return FileInfo::FileType::kArchives;
    } else if (backupMimeTypes.contains(mimeType)) {
        return FileInfo::FileType::kBackups;
    } else {
        return FileInfo::FileType::kUnknown;
    }
}

QString MimeTypeDisplayManager::defaultIcon(const QString &mimeType) const
{
    return defaultIconNames.value(displayNameToEnum(mimeType));
}

QMap<FileInfo::FileType, QString> MimeTypeDisplayManager::displayNames() const
{
    return displayNamesMap;
}

QStringList MimeTypeDisplayManager::readlines(const QString &path)
{
    QStringList result;
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return result;
    }
    QTextStream in(&file);
    while (!in.atEnd()) {
        // Read new line
        QString line = in.readLine();
        // Skip empty line or line with invalid format
        if (line.trimmed().isEmpty()) {
            continue;
        }
        result.append(line.trimmed());
    }
    file.close();
    return result;
}

void MimeTypeDisplayManager::loadSupportMimeTypes()
{
    QString textPath = QString("%1/%2").arg(StandardPaths::location(StandardPaths::kMimeTypePath), "text.mimetype");
    QString archivePath = QString("%1/%2").arg(StandardPaths::location(StandardPaths::kMimeTypePath), "archive.mimetype");
    QString videoPath = QString("%1/%2").arg(StandardPaths::location(StandardPaths::kMimeTypePath), "video.mimetype");
    QString audioPath = QString("%1/%2").arg(StandardPaths::location(StandardPaths::kMimeTypePath), "audio.mimetype");
    QString imagePath = QString("%1/%2").arg(StandardPaths::location(StandardPaths::kMimeTypePath), "image.mimetype");
    QString executablePath = QString("%1/%2").arg(StandardPaths::location(StandardPaths::kMimeTypePath), "executable.mimetype");
    QString backupPath = QString("%1/%2").arg(StandardPaths::location(StandardPaths::kMimeTypePath), "backup.mimetype");
    textMimeTypes = readlines(textPath);
    archiveMimeTypes = readlines(archivePath);
    videoMimeTypes = readlines(videoPath);
    audioMimeTypes = readlines(audioPath);
    imageMimeTypes = readlines(imagePath);
    executableMimeTypes = readlines(executablePath);
    backupMimeTypes = readlines(backupPath);
}

QStringList MimeTypeDisplayManager::supportArchiveMimetypes() const
{
    return archiveMimeTypes;
}

QStringList MimeTypeDisplayManager::supportVideoMimeTypes() const
{
    return videoMimeTypes;
}

MimeTypeDisplayManager *MimeTypeDisplayManager::instance()
{
    static MimeTypeDisplayManager *ins = new MimeTypeDisplayManager();
    return ins;
}

QStringList MimeTypeDisplayManager::supportAudioMimeTypes() const
{
    return audioMimeTypes;
}
