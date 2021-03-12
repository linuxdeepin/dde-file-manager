/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     max-lv<lvwujun@uniontech.com>
 *
 * Maintainer: dengkeyun<dengkeyun@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include "dfmstandardpaths.h"

QStringList MimeTypeDisplayManager::ArchiveMimeTypes;
QStringList MimeTypeDisplayManager::TextMimeTypes;
QStringList MimeTypeDisplayManager::VideoMimeTypes;
QStringList MimeTypeDisplayManager::AudioMimeTypes;
QStringList MimeTypeDisplayManager::ImageMimeTypes;
QStringList MimeTypeDisplayManager::ExecutableMimeTypes;
QStringList MimeTypeDisplayManager::BackupMimeTypes;


MimeTypeDisplayManager::MimeTypeDisplayManager(QObject *parent) : QObject(parent)
{
    initData();
    initConnect();
}

MimeTypeDisplayManager::~MimeTypeDisplayManager()
{

}

void MimeTypeDisplayManager::initData()
{
    m_displayNames[FileType::Directory] = tr("Directory");
    m_displayNames[FileType::DesktopApplication] = tr("Application");
    m_displayNames[FileType::Videos] = tr("Video");
    m_displayNames[FileType::Audios] = tr("Audio");
    m_displayNames[FileType::Images] = tr("Image");
    m_displayNames[FileType::Archives] = tr("Archive");
    m_displayNames[FileType::Documents] = tr("Text");
    m_displayNames[FileType::Executable] = tr("Executable");
    m_displayNames[FileType::Backups] = tr("Backup file");
    m_displayNames[FileType::Unknown] = tr("Unknown");


    m_defaultIconNames[FileType::Directory] = "folder";
    m_defaultIconNames[FileType::DesktopApplication] = "application-default-icon";
    m_defaultIconNames[FileType::Videos] = "video";
    m_defaultIconNames[FileType::Audios] = "music";
    m_defaultIconNames[FileType::Images] = "image";
    m_defaultIconNames[FileType::Archives] = "application-x-archive";
    m_defaultIconNames[FileType::Documents] = "text-plain";
    m_defaultIconNames[FileType::Executable] = "application-x-executable";
    m_defaultIconNames[FileType::Backups] = "application-x-archive"; // generic backup file icon?
    m_defaultIconNames[FileType::Unknown] = "application-default-icon";

    loadSupportMimeTypes();
}

void MimeTypeDisplayManager::initConnect()
{

}

QString MimeTypeDisplayManager::displayName(const QString &mimeType)
{
#ifdef QT_DEBUG
    return m_displayNames.value(displayNameToEnum(mimeType)) + " (" + mimeType + ")";
#endif // Q_DEBUG
    return m_displayNames.value(displayNameToEnum(mimeType));
}

DAbstractFileInfo::FileType MimeTypeDisplayManager::displayNameToEnum(const QString &mimeType)
{
    if (mimeType == "application/x-desktop") {
        return FileType::DesktopApplication;
    } else if (mimeType == "inode/directory") {
        return FileType::Directory;
    } else if (mimeType == "application/x-executable" || ExecutableMimeTypes.contains(mimeType)) {
        return FileType::Executable;
    } else if (mimeType.startsWith("video/") || VideoMimeTypes.contains(mimeType)) {
        return FileType::Videos;
    } else if (mimeType.startsWith("audio/") || AudioMimeTypes.contains(mimeType)) {
        return FileType::Audios;
    } else if (mimeType.startsWith("image/") || ImageMimeTypes.contains(mimeType)) {
        return FileType::Images;
    } else if (mimeType.startsWith("text/") || TextMimeTypes.contains(mimeType)) {
        return FileType::Documents;
    } else if (ArchiveMimeTypes.contains(mimeType)) {
        return FileType::Archives;
    } else if (BackupMimeTypes.contains(mimeType)) {
        return FileType::Backups;
    } else {
        return FileType::Unknown;
    }
}

QString MimeTypeDisplayManager::defaultIcon(const QString &mimeType)
{
    return m_defaultIconNames.value(displayNameToEnum(mimeType));
}

QMap<DAbstractFileInfo::FileType, QString> MimeTypeDisplayManager::displayNames()
{
    return m_displayNames;
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
    QString textPath = QString("%1/%2").arg(DFMStandardPaths::location(DFMStandardPaths::MimeTypePath), "text.mimetype");
    QString archivePath = QString("%1/%2").arg(DFMStandardPaths::location(DFMStandardPaths::MimeTypePath), "archive.mimetype");
    QString videoPath = QString("%1/%2").arg(DFMStandardPaths::location(DFMStandardPaths::MimeTypePath), "video.mimetype");
    QString audioPath = QString("%1/%2").arg(DFMStandardPaths::location(DFMStandardPaths::MimeTypePath), "audio.mimetype");
    QString imagePath = QString("%1/%2").arg(DFMStandardPaths::location(DFMStandardPaths::MimeTypePath), "image.mimetype");
    QString executablePath = QString("%1/%2").arg(DFMStandardPaths::location(DFMStandardPaths::MimeTypePath), "executable.mimetype");
    QString backupPath = QString("%1/%2").arg(DFMStandardPaths::location(DFMStandardPaths::MimeTypePath), "backup.mimetype");
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
