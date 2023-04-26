// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mimetypedisplaymanager.h"

#include <dfm-base/base/standardpaths.h>

#include <QFile>
#include <QTextStream>
#include <QDebug>

using namespace dfmbase;

MimeTypeDisplayManager::MimeTypeDisplayManager(QObject *parent)
    : QObject(parent)
{
    initData();
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

QString MimeTypeDisplayManager::displayName(const QString &mimeType)
{
#ifdef QT_DEBUG
    return displayNamesMap.value(displayNameToEnum(mimeType)) + " (" + mimeType + ")";
#endif   // Q_DEBUG
    return displayNamesMap.value(displayNameToEnum(mimeType));
}

FileInfo::FileType MimeTypeDisplayManager::displayNameToEnum(const QString &mimeType)
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

QString MimeTypeDisplayManager::defaultIcon(const QString &mimeType)
{
    return defaultIconNames.value(displayNameToEnum(mimeType));
}

QMap<FileInfo::FileType, QString> MimeTypeDisplayManager::displayNames()
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

QStringList MimeTypeDisplayManager::supportArchiveMimetypes()
{
    return archiveMimeTypes;
}

QStringList MimeTypeDisplayManager::supportVideoMimeTypes()
{
    return videoMimeTypes;
}

MimeTypeDisplayManager *MimeTypeDisplayManager::instance()
{
    static MimeTypeDisplayManager ins;
    return &ins;
}

QStringList MimeTypeDisplayManager::supportAudioMimeTypes()
{
    return audioMimeTypes;
}
