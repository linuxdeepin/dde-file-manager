/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyu@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#include "localfileinfo.h"
#include "private/localfileinfo_p.h"
#include "base/urlroute.h"
#include "shutil/fileutils.h"
#include "dfileiconprovider.h"
#include "base/standardpaths.h"

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

/*!
 * \class LocalFileInfo 本地文件信息类
 * \brief 内部实现本地文件的fileinfo，对应url的scheme是file://
 */
DFMBASE_BEGIN_NAMESPACE

LocalFileInfo::LocalFileInfo(const QUrl &url)
    : AbstractFileInfo (url),
      d(new LocalFileInfoPrivate(this))
{

}

LocalFileInfo::~LocalFileInfo()
{

}

LocalFileInfo& LocalFileInfo::operator =(const LocalFileInfo &info)
{
    AbstractFileInfo::operator=(info);
    d->icon = info.d->icon;
    d->inode = info.d->inode;
    d->mimeType = info.d->mimeType;
    return *this;
}
/*!
 * \brief isBlockDev 获取是否是块设备
 *
 * \return bool 是否是块设备
 */
bool LocalFileInfo::isBlockDev() const
{
    return fileType() == MimeDatabase::FileType::BlockDevice;
}
/*!
 * \brief mountPath 获取挂载路径
 *
 * \return QString 挂载路径
 */
QString LocalFileInfo::mountPath() const
{
    if(!isBlockDev()) return "";
    else return "";
}
/*!
 * \brief isCharDev 获取是否是字符设备
 *
 * \return bool 是否是字符设备
 */
bool LocalFileInfo::isCharDev() const
{
    return fileType() == MimeDatabase::FileType::CharDevice;
}
/*!
 * \brief isFifo 获取当前是否为管道文件
 *
 * \return bool 返回当前是否为管道文件
 */
bool LocalFileInfo::isFifo() const
{
    return fileType() == MimeDatabase::FileType::FIFOFile;
}
/*!
 * \brief isSocket 获取当前是否为套接字文件
 *
 * \return bool 返回是否为套接字文件
 */
bool LocalFileInfo::isSocket() const
{
    return fileType() == MimeDatabase::FileType::SocketFile;
}
/*!
 * \brief isRegular 获取当前是否是常规文件(与isFile一致)
 *
 * \return bool 返回是否是常规文件(与isFile一致)
 */
bool LocalFileInfo::isRegular() const
{
    return fileType() == MimeDatabase::FileType::RegularFile;
}
/*!
 * \brief isRegular 设置自定义角标信息
 *
 * \param LocalFileInfo::DFMEmblemInfos 自定义扩展信息
 */
void LocalFileInfo::setEmblems(const LocalFileInfo::DFMEmblemInfos &infos)
{
    d->EmblemsInfo = infos;
}
/*!
 * \brief clearEmblems 清除自定义角标
 */
void LocalFileInfo::clearEmblems()
{
    d->EmblemsInfo.clear();
}
/*!
 * \brief emblems 角标信息设置
 *
 * \return LocalFileInfo::DFMEmblemInfos 自定义信息
 */
LocalFileInfo::DFMEmblemInfos LocalFileInfo::emblems() const
{
    return d->EmblemsInfo;
}
/*!
 * \brief setIcon 设置文件图标
 *
 * \param const QIcon &icon 文件图标
 */
void LocalFileInfo::setIcon(const QIcon &icon)
{
    d->icon = icon;
}
/*!
 * \brief setIcon 获取文件图标
 *
 * \return QIcon 文件图标
 */
QIcon LocalFileInfo::icon() const
{
    if (d->icon.isNull()) {
        if (isDir())
            return QIcon::fromTheme("folder");

        if (isFile())
            //如果是文件就使用mimeType初始化图标
            return QIcon(d->mimeType.iconName());
    }

    return d->icon;
}
/*!
 * \brief fileType 获取文件类型
 *
 * \return DMimeDatabase::FileType 文件设备类型
 */
MimeDatabase::FileType LocalFileInfo::fileType() const
{
    // fix bug#52950 【专业版1030】【文管5.2.0.72】回收站删除指向块设备的链接文件时，删除失败
    // QT_STATBUF判断链接文件属性时，判断的是指向文件的属性，使用QFileInfo判断
    if (d->cacheFileType != MimeDatabase::FileType::Unknown)
        return d->cacheFileType;

    QString absoluteFilePath = filePath();
    if (absoluteFilePath.startsWith(StandardPaths::location(StandardPaths::TrashFilesPath))
            && isSymLink()) {
        d->cacheFileType = MimeDatabase::FileType::RegularFile;
        return d->cacheFileType;
    }

    // Cannot access statBuf.st_mode from the filesystem engine, so we have to stat again.
    // In addition we want to follow symlinks.
    const QByteArray &nativeFilePath = QFile::encodeName(absoluteFilePath);
    QT_STATBUF statBuffer;
    if (QT_STAT(nativeFilePath.constData(), &statBuffer) == 0) {
        if (S_ISDIR(statBuffer.st_mode))
            d->cacheFileType = MimeDatabase::FileType::Directory;

        else if (S_ISCHR(statBuffer.st_mode))
            d->cacheFileType = MimeDatabase::FileType::CharDevice;

        else if (S_ISBLK(statBuffer.st_mode))
            d->cacheFileType = MimeDatabase::FileType::BlockDevice;

        else if (S_ISFIFO(statBuffer.st_mode))
            d->cacheFileType = MimeDatabase::FileType::FIFOFile;

        else if (S_ISSOCK(statBuffer.st_mode))
            d->cacheFileType = MimeDatabase::FileType::SocketFile;

        else if (S_ISREG(statBuffer.st_mode))
            d->cacheFileType = MimeDatabase::FileType::RegularFile;
    }

    return d->cacheFileType;
}
/*!
 * \brief linkTargetPath 获取链接文件的目标路径
 *
 * \return QString 链接文件的目标文件路径
 */
QString LocalFileInfo::linkTargetPath() const
{
    return AbstractFileInfo::symLinkTarget();
}
/*!
 * \brief countChildFile 文件夹下子文件的个数，只统计下一层不递归
 *
 * \return int 子文件个数
 */
int LocalFileInfo::countChildFile() const
{
    if (isDir()) {
        QDir dir(absoluteFilePath());
        QStringList entryList = dir.entryList(QDir::AllEntries | QDir::System
                                              | QDir::NoDotAndDotDot | QDir::Hidden);
        return entryList.size();
    }

    return -1;
}
/*!
 * \brief sizeFormat 格式化大小
 *
 * \return QString 大小格式化后的大小
 */
QString LocalFileInfo::sizeFormat() const
{
    if (isDir()) {
        return QStringLiteral("-");
    }

    qreal fileSize(AbstractFileInfo::size());
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
}
/*!
 * \brief fileDisplayName 文件的显示名称，一般为文件的名称
 *
 * \return QString 文件的显示名称
 */
QString LocalFileInfo::fileDisplayName() const
{
    return fileName();
}
/*!
 * \brief refresh 刷新文件信息
 */
void LocalFileInfo::refresh()
{
    return AbstractFileInfo::refresh();
}
/*!
 * \brief toQFileInfo 获取他的QFileInfo实例对象
 *
 * \return QFileInfo 文件的QFileInfo实例
 */
QFileInfo LocalFileInfo::toQFileInfo() const
{
    return QFileInfo(d->url.path());
}
/*!
 * \brief extraProperties 获取文件的扩展属性
 *
 * \return QVariantHash 文件的扩展属性Hash
 */
QVariantHash LocalFileInfo::extraProperties() const
{
    return d->extraProperties;
}
/*!
 * \brief inode linux系统下的唯一表示符
 *
 * \return quint64 文件的inode
 */
quint64 LocalFileInfo::inode() const
{
    if (d->inode != 0) {
        return d->inode;
    }

    struct stat statinfo;
    int filestat = stat(absoluteFilePath().toStdString().c_str(), &statinfo);
    if (filestat != 0) {
        return 0;
    }
    d->inode = statinfo.st_ino;

    return d->inode;
}

DFMBASE_END_NAMESPACE
