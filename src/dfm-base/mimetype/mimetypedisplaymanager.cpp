// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mimetypedisplaymanager.h"

#include "dfm-base/base/standardpaths.h"

#include <QFile>
#include <QTextStream>
#include <QDebug>

using namespace dfmbase;


MimeTypeDisplayManager *MimeTypeDisplayManager::self { nullptr };

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
    displayNamesMap[AbstractFileInfo::FileType::kDirectory] = tr("Directory");
    displayNamesMap[AbstractFileInfo::FileType::kDesktopApplication] = tr("Application");
    displayNamesMap[AbstractFileInfo::FileType::kVideos] = tr("Video");
    displayNamesMap[AbstractFileInfo::FileType::kAudios] = tr("Audio");
    displayNamesMap[AbstractFileInfo::FileType::kImages] = tr("Image");
    displayNamesMap[AbstractFileInfo::FileType::kArchives] = tr("Archive");
    displayNamesMap[AbstractFileInfo::FileType::kDocuments] = tr("Text");
    displayNamesMap[AbstractFileInfo::FileType::kExecutable] = tr("Executable");
    displayNamesMap[AbstractFileInfo::FileType::kBackups] = tr("Backup file");
    displayNamesMap[AbstractFileInfo::FileType::kUnknown] = tr("Unknown");

    defaultIconNames[AbstractFileInfo::FileType::kDirectory] = "folder";
    defaultIconNames[AbstractFileInfo::FileType::kDesktopApplication] = "application-default-icon";
    defaultIconNames[AbstractFileInfo::FileType::kVideos] = "video";
    defaultIconNames[AbstractFileInfo::FileType::kAudios] = "music";
    defaultIconNames[AbstractFileInfo::FileType::kImages] = "image";
    defaultIconNames[AbstractFileInfo::FileType::kArchives] = "application-x-archive";
    defaultIconNames[AbstractFileInfo::FileType::kDocuments] = "text-plain";
    defaultIconNames[AbstractFileInfo::FileType::kExecutable] = "application-x-executable";
    defaultIconNames[AbstractFileInfo::FileType::kBackups] = "application-x-archive";   // generic backup file icon?
    defaultIconNames[AbstractFileInfo::FileType::kUnknown] = "application-default-icon";

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

AbstractFileInfo::FileType MimeTypeDisplayManager::displayNameToEnum(const QString &mimeType)
{
    if (mimeType == "application/x-desktop") {
        return AbstractFileInfo::FileType::kDesktopApplication;
    } else if (mimeType == "inode/directory") {
        return AbstractFileInfo::FileType::kDirectory;
    } else if (mimeType == "application/x-executable" || ExecutableMimeTypes.contains(mimeType)) {
        return AbstractFileInfo::FileType::kExecutable;
    } else if (mimeType.startsWith("video/") || VideoMimeTypes.contains(mimeType)) {
        return AbstractFileInfo::FileType::kVideos;
    } else if (mimeType.startsWith("audio/") || AudioMimeTypes.contains(mimeType)) {
        return AbstractFileInfo::FileType::kAudios;
    } else if (mimeType.startsWith("image/") || ImageMimeTypes.contains(mimeType)) {
        return AbstractFileInfo::FileType::kImages;
    } else if (mimeType.startsWith("text/") || TextMimeTypes.contains(mimeType)) {
        return AbstractFileInfo::FileType::kDocuments;
    } else if (ArchiveMimeTypes.contains(mimeType)) {
        return AbstractFileInfo::FileType::kArchives;
    } else if (BackupMimeTypes.contains(mimeType)) {
        return AbstractFileInfo::FileType::kBackups;
    } else {
        return AbstractFileInfo::FileType::kUnknown;
    }
}

QString MimeTypeDisplayManager::defaultIcon(const QString &mimeType)
{
    return defaultIconNames.value(displayNameToEnum(mimeType));
}

QMap<AbstractFileInfo::FileType, QString> MimeTypeDisplayManager::displayNames()
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

MimeTypeDisplayManager *MimeTypeDisplayManager::instance()
{
    if (!self)
        self = new MimeTypeDisplayManager();
    return self;
}

QStringList MimeTypeDisplayManager::supportAudioMimeTypes()
{
    return AudioMimeTypes;
}
