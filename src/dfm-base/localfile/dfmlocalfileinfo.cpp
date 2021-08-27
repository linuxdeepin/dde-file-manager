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

#include "dfmlocalfileinfo.h"
#include "private/dfmlocalfileinfo_p.h"

#include "base/dfmurlroute.h"
#include "shutil/fileutils.h"
#include "localfile/filecontroller.h"
#include "base/singleton.hpp"
#include "dthumbnailprovider.h"
#include "dfileiconprovider.h"
#include "shutil/dmimedatabase.h"
#include "base/dfmapplication.h"
#include "base/dfmstandardpaths.h"


#ifdef SW_LABEL
#include "sw_label/filemanagerlibrary.h"
#endif

#include <QDateTime>
#include <QDir>
#include <QPainter>
#include <QApplication>
#include <QtConcurrent>
#include <qplatformdefs.h>

#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <mntent.h>

DFMLocalFileInfo::DFMLocalFileInfo()
    :d_ptr(new DFMLocalFileInfoPrivate(this))
{

}

DFMLocalFileInfo::DFMLocalFileInfo(const QString &filePath)
    : DFMLocalFileInfo(QUrl::fromLocalFile(filePath))
{

}

DFMLocalFileInfo::DFMLocalFileInfo(const QUrl &url)
    : DAbstractFileInfo (url),
      d_ptr(new DFMLocalFileInfoPrivate(this))
{

}

DFMLocalFileInfo::DFMLocalFileInfo(const QFileInfo &fileInfo)
    : DFMLocalFileInfo(QUrl::fromLocalFile(fileInfo.absoluteFilePath()))
{

}

DFMLocalFileInfo::~DFMLocalFileInfo()
{

}

DFMLocalFileInfo& DFMLocalFileInfo::operator =(const DFMLocalFileInfo &info)
{
    Q_D(DFMLocalFileInfo);
    DAbstractFileInfo::operator=(info);
    d->icon = info.d_ptr->icon;
    d->inode = info.d_ptr->inode;
    d->mimeType = info.d_ptr->mimeType;
    return *this;
}

bool DFMLocalFileInfo::isBlockDev() const
{
    const QByteArray &nativeFilePath = QFile::encodeName(filePath());
    QT_STATBUF statBuffer;
    if (QT_STAT(nativeFilePath.constData(), &statBuffer) == 0) {
        if (S_ISBLK(statBuffer.st_mode))
            return true;
        else
            return false;
    }
    return false;
}

QString DFMLocalFileInfo::mountPath() const
{
    if(!isBlockDev()) return "";
    else return "";
}

bool DFMLocalFileInfo::isCharDev() const
{
    const QByteArray &nativeFilePath = QFile::encodeName(filePath());
    QT_STATBUF statBuffer;
    if (QT_STAT(nativeFilePath.constData(), &statBuffer) == 0) {
        if (S_ISCHR(statBuffer.st_mode))
            return true;
        else
            return false;
    }
    return false;
}

bool DFMLocalFileInfo::isFifo() const
{
    const QByteArray &nativeFilePath = QFile::encodeName(filePath());
    QT_STATBUF statBuffer;
    if (QT_STAT(nativeFilePath.constData(), &statBuffer) == 0) {
        if (S_ISFIFO(statBuffer.st_mode))
            return true;
        else
            return false;
    }
    return false;
}

bool DFMLocalFileInfo::isSocket() const
{
    const QByteArray &nativeFilePath = QFile::encodeName(filePath());
    QT_STATBUF statBuffer;
    if (QT_STAT(nativeFilePath.constData(), &statBuffer) == 0) {
        if (S_ISSOCK(statBuffer.st_mode))
            return true;
        else
            return false;
    }
    return false;
}

bool DFMLocalFileInfo::isRegular() const
{
    const QByteArray &nativeFilePath = QFile::encodeName(filePath());
    QT_STATBUF statBuffer;
    if (QT_STAT(nativeFilePath.constData(), &statBuffer) == 0) {
        if (S_ISREG(statBuffer.st_mode))
            return true;
        else
            return false;
    }
    return false;
}

void DFMLocalFileInfo::setEmblems(const DFMLocalFileInfo::DFMEmblemInfos &infos)
{
    Q_D(DFMLocalFileInfo);
    d->EmblemsInfo = infos;
}

void DFMLocalFileInfo::clearEmblems()
{
    Q_D(DFMLocalFileInfo);
    d->EmblemsInfo.clear();
}

DFMLocalFileInfo::DFMEmblemInfos DFMLocalFileInfo::emblems() const
{
    Q_D(const DFMLocalFileInfo);
    return d->EmblemsInfo;
}

void DFMLocalFileInfo::setIcon(const QIcon &icon)
{
    Q_D(DFMLocalFileInfo);
    d->icon = icon;
}

QIcon DFMLocalFileInfo::icon() const
{
    Q_D(const DFMLocalFileInfo);

    if (d->icon.isNull()) {
        QFileInfo localFileInfo(filePath());
        if (localFileInfo.isDir())
            return QIcon::fromTheme("folder");

        if (localFileInfo.isFile())
            //如果是文件就使用mimeType初始化图标
            return QIcon(d->mimeType.iconName());
    }

    return d->icon;
}


DMimeDatabase::FileType DFMLocalFileInfo::fileType() const
{
    Q_D(const DFMLocalFileInfo);

    // fix bug#52950 【专业版1030】【文管5.2.0.72】回收站删除指向块设备的链接文件时，删除失败
    // QT_STATBUF判断链接文件属性时，判断的是指向文件的属性，使用QFileInfo判断
    QString absoluteFilePath = d->fileInfo.absoluteFilePath();
    if (absoluteFilePath.startsWith(DFMStandardPaths::location(DFMStandardPaths::TrashFilesPath))
            && d->fileInfo.isSymLink()) {
        return DMimeDatabase::FileType::RegularFile;
    }

    // Cannot access statBuf.st_mode from the filesystem engine, so we have to stat again.
    // In addition we want to follow symlinks.
    const QByteArray &nativeFilePath = QFile::encodeName(absoluteFilePath);
    QT_STATBUF statBuffer;
    if (QT_STAT(nativeFilePath.constData(), &statBuffer) == 0) {
        if (S_ISDIR(statBuffer.st_mode))
            return DMimeDatabase::FileType::Directory;

        if (S_ISCHR(statBuffer.st_mode))
            return DMimeDatabase::FileType::CharDevice;

        if (S_ISBLK(statBuffer.st_mode))
            return DMimeDatabase::FileType::BlockDevice;

        if (S_ISFIFO(statBuffer.st_mode))
            return DMimeDatabase::FileType::FIFOFile;

        if (S_ISSOCK(statBuffer.st_mode))
            return DMimeDatabase::FileType::SocketFile;

        if (S_ISREG(statBuffer.st_mode))
            return DMimeDatabase::FileType::RegularFile;
    }

    return DMimeDatabase::FileType::Unknown;
}

QString DFMLocalFileInfo::linkTargetPath() const
{
    if (isSymLink()) {
        char buf[PATH_MAX];
        ::readlink(this->path().toLocal8Bit().data(), buf, PATH_MAX);
        if (QFileInfo(buf).exists()) return buf;
    }
    return "";
}

int DFMLocalFileInfo::countChildFile() const
{

    if (isDir()) {
        QDir dir(absoluteFilePath());
        QStringList entryList = dir.entryList(QDir::AllEntries | QDir::System
                                              | QDir::NoDotAndDotDot | QDir::Hidden);
        return entryList.size();
    }

    return -1;
}

QString DFMLocalFileInfo::sizeFormat() const
{
    Q_D(const DFMLocalFileInfo);

    if (isDir()) {
        return QStringLiteral("-");
    }

    qreal fileSize(DAbstractFileInfo::size());
    bool withUnitVisible = true;
    int forceUnit = -1;

    if (fileSize < 0) {
        qWarning() << "Negative number passed to formatSize():" << fileSize;
        fileSize = 0;
    }

    bool isForceUnit = false;
    QStringList list{" B" , " KB" , " MB" , " GB" , " TB"};

    QStringListIterator i(list);
    QString unit = i.hasNext() ? i.next() : QStringLiteral(" B");

    int index = 0;
    while (i.hasNext()) {
        if (fileSize < 1024 && !isForceUnit) {
            break;
        }

        if (isForceUnit && index == forceUnit) {
            break;
        }

        unit = i.next();
        fileSize /= 1024;
        index++;
    }
    QString unitString = withUnitVisible ? unit : QString();
    return QString("%1%2").arg(d->sizeString(QString::number(fileSize, 'f', 1)), unitString);

    return FileUtils::formatSize(size());
}



QString DFMLocalFileInfo::fileDisplayName() const
{
    return fileName();
}

void DFMLocalFileInfo::refresh()
{
    return DAbstractFileInfo::refresh();
}

//QString DFMLocalFileInfo::iconName() const
//{
//    return DAbstractFileInfo::iconName();
//}

QFileInfo DFMLocalFileInfo::toQFileInfo() const
{
    Q_D(const DFMLocalFileInfo);

    return d->fileInfo;
}

QVariantHash DFMLocalFileInfo::extraProperties() const
{
    Q_D(const DFMLocalFileInfo);

    return d->extraProperties;
}

quint64 DFMLocalFileInfo::inode() const
{
    Q_D(const DFMLocalFileInfo);
    if (d->inode != 0) {
        return d->inode;
    }

    struct stat statinfo;
    int filestat = stat(d->fileInfo.absoluteFilePath().toStdString().c_str(), &statinfo);
    if (filestat != 0) {
        return 0;
    }
    d->inode = statinfo.st_ino;

    return d->inode;
}
