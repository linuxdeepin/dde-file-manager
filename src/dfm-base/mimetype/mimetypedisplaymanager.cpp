/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lanxuesong<lanxuesong@uniontech.com>
 *
 * Maintainer: liyigang<liyigang@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "mimetypedisplaymanager.h"

#include "dfm-base/base/standardpaths.h"

#include <QFile>
#include <QTextStream>
#include <QDebug>

DFMBASE_USE_NAMESPACE

QStringList MimeTypeDisplayManager::ArchiveMimeTypes;
QStringList MimeTypeDisplayManager::TextMimeTypes;
QStringList MimeTypeDisplayManager::VideoMimeTypes;
QStringList MimeTypeDisplayManager::AudioMimeTypes;
QStringList MimeTypeDisplayManager::ImageMimeTypes;
QStringList MimeTypeDisplayManager::ExecutableMimeTypes;
QStringList MimeTypeDisplayManager::BackupMimeTypes;

MimeTypeDisplayManager::MimeTypeDisplayManager(QObject *parent)
    : QObject(parent)
{
    initData();
    initConnect();
}

MimeTypeDisplayManager::~MimeTypeDisplayManager()
{
}

void MimeTypeDisplayManager::initData()
{
    displayNamesMap[FileType::kDirectory] = tr("Directory");
    displayNamesMap[FileType::kDesktopApplication] = tr("Application");
    displayNamesMap[FileType::kVideos] = tr("Video");
    displayNamesMap[FileType::kAudios] = tr("Audio");
    displayNamesMap[FileType::kImages] = tr("Image");
    displayNamesMap[FileType::kArchives] = tr("Archive");
    displayNamesMap[FileType::kDocuments] = tr("Text");
    displayNamesMap[FileType::kExecutable] = tr("Executable");
    displayNamesMap[FileType::kBackups] = tr("Backup file");
    displayNamesMap[FileType::kUnknown] = tr("Unknown");

    defaultIconNames[FileType::kDirectory] = "folder";
    defaultIconNames[FileType::kDesktopApplication] = "application-default-icon";
    defaultIconNames[FileType::kVideos] = "video";
    defaultIconNames[FileType::kAudios] = "music";
    defaultIconNames[FileType::kImages] = "image";
    defaultIconNames[FileType::kArchives] = "application-x-archive";
    defaultIconNames[FileType::kDocuments] = "text-plain";
    defaultIconNames[FileType::kExecutable] = "application-x-executable";
    defaultIconNames[FileType::kBackups] = "application-x-archive";   // generic backup file icon?
    defaultIconNames[FileType::kUnknown] = "application-default-icon";

    loadSupportMimeTypes();
}

void MimeTypeDisplayManager::initConnect()
{
}

QString MimeTypeDisplayManager::displayName(const QString &mimeType)
{
#ifdef QT_DEBUG
    return displayNamesMap.value(displayNameToEnum(mimeType)) + " (" + mimeType + ")";
#endif   // Q_DEBUG
    return displayNamesMap.value(displayNameToEnum(mimeType));
}

AbstractFileInfo::Type MimeTypeDisplayManager::displayNameToEnum(const QString &mimeType)
{
    if (mimeType == "application/x-desktop") {
        return FileType::kDesktopApplication;
    } else if (mimeType == "inode/directory") {
        return FileType::kDirectory;
    } else if (mimeType == "application/x-executable" || ExecutableMimeTypes.contains(mimeType)) {
        return FileType::kExecutable;
    } else if (mimeType.startsWith("video/") || VideoMimeTypes.contains(mimeType)) {
        return FileType::kVideos;
    } else if (mimeType.startsWith("audio/") || AudioMimeTypes.contains(mimeType)) {
        return FileType::kAudios;
    } else if (mimeType.startsWith("image/") || ImageMimeTypes.contains(mimeType)) {
        return FileType::kImages;
    } else if (mimeType.startsWith("text/") || TextMimeTypes.contains(mimeType)) {
        return FileType::kDocuments;
    } else if (ArchiveMimeTypes.contains(mimeType)) {
        return FileType::kArchives;
    } else if (BackupMimeTypes.contains(mimeType)) {
        return FileType::kBackups;
    } else {
        return FileType::kUnknown;
    }
}

QString MimeTypeDisplayManager::defaultIcon(const QString &mimeType)
{
    return defaultIconNames.value(displayNameToEnum(mimeType));
}

QMap<AbstractFileInfo::Type, QString> MimeTypeDisplayManager::displayNames()
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
    TextMimeTypes = readlines(textPath);
    ArchiveMimeTypes = readlines(archivePath);
    VideoMimeTypes = readlines(videoPath);
    AudioMimeTypes = readlines(audioPath);
    ImageMimeTypes = readlines(imagePath);
    ExecutableMimeTypes = readlines(executablePath);
    BackupMimeTypes = readlines(backupPath);
}

QStringList MimeTypeDisplayManager::supportArchiveMimetypes()
{
    return ArchiveMimeTypes;
}

QStringList MimeTypeDisplayManager::supportVideoMimeTypes()
{
    return VideoMimeTypes;
}
