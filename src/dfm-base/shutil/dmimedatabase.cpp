/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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

#include "shutil/dmimedatabase.h"
#include "shutil/fileutils.h"

#include "base/dfmstandardpaths.h"

#include <QFileInfo>
#include <QRegularExpression>
#include <QMimeDatabase>
#include <QString>
#include <QHash>
#include <QtConcurrent>

//global mimedatabase cache
Q_GLOBAL_STATIC(QMimeDatabase, mimedb)

//global mimetypes define list cache
Q_GLOBAL_STATIC(QStringList, mimeArchiveTypes);
Q_GLOBAL_STATIC(QStringList, mimeTextTypes)
Q_GLOBAL_STATIC(QStringList, mimeVideoTypes)
Q_GLOBAL_STATIC(QStringList, mimeAudioTypes)
Q_GLOBAL_STATIC(QStringList, mimeImageTypes)
Q_GLOBAL_STATIC(QStringList, mimeExecutableTypes)
Q_GLOBAL_STATIC(QStringList, mimeBackupTypes)

typedef QHash<int,QString> kiVsHash;
Q_GLOBAL_STATIC(kiVsHash, mimeDisplayNames)
Q_GLOBAL_STATIC(kiVsHash, mimeStdIconNames)

QStringList readlines(const QString &path)
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

bool loadSupportMimeTypes()
{
    QString textPath = QString("%1/%2").arg(DFMStandardPaths::location(DFMStandardPaths::MimeTypePath), "text.mimetype");
    QString archivePath = QString("%1/%2").arg(DFMStandardPaths::location(DFMStandardPaths::MimeTypePath), "archive.mimetype");
    QString videoPath = QString("%1/%2").arg(DFMStandardPaths::location(DFMStandardPaths::MimeTypePath), "video.mimetype");
    QString audioPath = QString("%1/%2").arg(DFMStandardPaths::location(DFMStandardPaths::MimeTypePath), "audio.mimetype");
    QString imagePath = QString("%1/%2").arg(DFMStandardPaths::location(DFMStandardPaths::MimeTypePath), "image.mimetype");
    QString executablePath = QString("%1/%2").arg(DFMStandardPaths::location(DFMStandardPaths::MimeTypePath), "executable.mimetype");
    QString backupPath = QString("%1/%2").arg(DFMStandardPaths::location(DFMStandardPaths::MimeTypePath), "backup.mimetype");
    *mimeTextTypes = readlines(textPath);
    *mimeArchiveTypes = readlines(archivePath);
    *mimeVideoTypes = readlines(videoPath);
    *mimeAudioTypes = readlines(audioPath);
    *mimeImageTypes = readlines(imagePath);
    *mimeExecutableTypes = readlines(executablePath);
    *mimeBackupTypes = readlines(backupPath);
    return true;
}

bool loadFileTypeName()
{
    (*mimeDisplayNames)[DMimeDatabase::FileType::Directory] = QObject::tr("Directory");
    (*mimeDisplayNames)[DMimeDatabase::FileType::DesktopApplication] = QObject::tr("Application");
    (*mimeDisplayNames)[DMimeDatabase::FileType::Videos] = QObject::tr("Video");
    (*mimeDisplayNames)[DMimeDatabase::FileType::Audios] = QObject::tr("Audio");
    (*mimeDisplayNames)[DMimeDatabase::FileType::Images] = QObject::tr("Image");
    (*mimeDisplayNames)[DMimeDatabase::FileType::Archives] = QObject::tr("Archive");
    (*mimeDisplayNames)[DMimeDatabase::FileType::Documents] = QObject::tr("Text");
    (*mimeDisplayNames)[DMimeDatabase::FileType::Executable] = QObject::tr("Executable");
    (*mimeDisplayNames)[DMimeDatabase::FileType::Backups] = QObject::tr("Backup file");
    (*mimeDisplayNames)[DMimeDatabase::FileType::Unknown] = QObject::tr("Unknown");
    return true;
}

bool loadMimeStdIcon()
{
    (*mimeStdIconNames)[DMimeDatabase::FileType::Directory] = "folder";
    (*mimeStdIconNames)[DMimeDatabase::FileType::DesktopApplication] = "application-default-icon";
    (*mimeStdIconNames)[DMimeDatabase::FileType::Videos] = "video";
    (*mimeStdIconNames)[DMimeDatabase::FileType::Audios] = "music";
    (*mimeStdIconNames)[DMimeDatabase::FileType::Images] = "image";
    (*mimeStdIconNames)[DMimeDatabase::FileType::Archives] = "application-x-archive";
    (*mimeStdIconNames)[DMimeDatabase::FileType::Documents] = "text-plain";
    (*mimeStdIconNames)[DMimeDatabase::FileType::Executable] = "application-x-executable";
    (*mimeStdIconNames)[DMimeDatabase::FileType::Backups] = "application-x-archive"; // generic backup file icon?
    (*mimeStdIconNames)[DMimeDatabase::FileType::Unknown] = "application-default-icon";

    return true;
}

//Q_GLOBAL_STATIC_WITH_ARGS(QFuture<void>, _threadLoadMimeTypes, {QtConcurrent::run(loadSupportMimeTypes)});
//Q_GLOBAL_STATIC_WITH_ARGS(QFuture<void>, _threadLoadStdIcons, {QtConcurrent::run(loadMimeStdIcon)});
//Q_GLOBAL_STATIC_WITH_ARGS(QFuture<void>, _threadLoadMimeNames, {QtConcurrent::run(loadFileTypeName)});

static auto _threadLoadMimeTypes = QtConcurrent::run(loadSupportMimeTypes);
static auto _threadLoadStdIcons = QtConcurrent::run(loadMimeStdIcon);
static auto _threadLoadMimeNames = QtConcurrent::run(loadFileTypeName);

DMimeDatabase::DMimeDatabase()
{
    Q_UNUSED(_threadLoadMimeTypes);
    Q_UNUSED(_threadLoadStdIcons);
    Q_UNUSED(_threadLoadMimeNames);
}

DMimeDatabase::~DMimeDatabase()
{

}

QString DMimeDatabase::mimeFileType(const QString &mimeType)
{
    Q_UNUSED(mimeType)
//todo:
//#ifdef QT_DEBUG
//    return mimeDisplayNames->value(mimeFileTypeNameToEnum(mimeType)) + " (" + mimeType + ")";
//#else // Q_DEBUG
//    return mimeDisplayNames->value(mimeFileTypeToEnum(mimeType));
//#endif
    return "";
}

DMimeDatabase::FileType DMimeDatabase::mimeFileTypeNameToEnum(const QString &mimeFileTypeName)
{
    if (mimeFileTypeName == "application/x-desktop") {
        return FileType::DesktopApplication;
    } else if (mimeFileTypeName == "inode/directory") {
        return FileType::Directory;
    } else if (mimeFileTypeName == "application/x-executable"
               || mimeExecutableTypes->contains(mimeFileTypeName)) {
        return FileType::Executable;
    } else if (mimeFileTypeName.startsWith("video/")
               || mimeVideoTypes->contains(mimeFileTypeName)) {
        return FileType::Videos;
    } else if (mimeFileTypeName.startsWith("audio/")
               || mimeAudioTypes->contains(mimeFileTypeName)) {
        return FileType::Audios;
    } else if (mimeFileTypeName.startsWith("image/")
               || mimeImageTypes->contains(mimeFileTypeName)) {
        return FileType::Images;
    } else if (mimeFileTypeName.startsWith("text/")
               || mimeTextTypes->contains(mimeFileTypeName)) {
        return FileType::Documents;
    } else if (mimeArchiveTypes->contains(mimeFileTypeName)) {
        return FileType::Archives;
    } else if (mimeBackupTypes->contains(mimeFileTypeName)) {
        return FileType::Backups;
    } else {
        return FileType::Unknown;
    }
}

QStringList DMimeDatabase::supportMimeFileType(DMimeDatabase::FileType mimeFileType)
{
    QStringList list;

    switch (mimeFileType) {
    case Documents:
        return list; //empty
    case Images:
        return *mimeImageTypes;
    case Videos:
        return *mimeVideoTypes;
    case Audios:
        return *mimeAudioTypes;
    case Archives:
        return *mimeArchiveTypes;
    case DesktopApplication:
        return list; //empty
    case Executable:
        return *mimeExecutableTypes;
    case Backups:
        return *mimeBackupTypes;
    default:
        return list; //empty
    }
}

QString DMimeDatabase::mimeStdIcon(const QString &mimeType)
{
    return mimeStdIconNames->value(mimeFileTypeNameToEnum(mimeType));
}

QMimeType DMimeDatabase::mimeTypeForFile(const QString &fileName, QMimeDatabase::MatchMode mode)
{
    return mimedb->mimeTypeForFile(fileName,mode);
}

QMimeType DMimeDatabase::mimeTypeForFile(const QFileInfo &fileInfo, QMimeDatabase::MatchMode mode)
{
    return mimedb->mimeTypeForFile(fileInfo,mode);
}

QList<QMimeType> DMimeDatabase::mimeTypesForFileName(const QString &fileName)
{
    return mimedb->mimeTypesForFileName(fileName);
}

QMimeType DMimeDatabase::mimeTypeForData(const QByteArray &data)
{
    return mimedb->mimeTypeForData(data);
}

QMimeType DMimeDatabase::mimeTypeForData(QIODevice *device)
{
    return mimedb->mimeTypeForData(device);
}

QMimeType DMimeDatabase::mimeTypeForUrl(const QUrl &url)
{
    if(url.isLocalFile())
        return mimedb->mimeTypeForUrl(url);
    else
        return mimedb->mimeTypeForFile(DFMUrlRoute::urlToPath(url));
}

QMimeType DMimeDatabase::mimeTypeForFileNameAndData(const QString &fileName, QIODevice *device)
{
    return mimedb->mimeTypeForFileNameAndData(fileName, device);
}

QMimeType DMimeDatabase::mimeTypeForFileNameAndData(const QString &fileName, const QByteArray &data)
{
    return mimedb->mimeTypeForFileNameAndData(fileName, data);
}

QString DMimeDatabase::suffixForFileName(const QString &fileName)
{
    return mimedb->suffixForFileName(fileName);
}

QList<QMimeType> DMimeDatabase::allMimeTypes()
{
    return mimedb->allMimeTypes();
}

