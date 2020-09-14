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

#include "dgvfsfileinfo.h"
#include "private/dgvfsfileinfo_p.h"
#include "app/define.h"

#include "shutil/fileutils.h"

#include "controllers/pathmanager.h"
#include "controllers/filecontroller.h"
#include "controllers/vaultcontroller.h"

#include "app/define.h"
#include "singleton.h"
#include "usershare/usersharemanager.h"
#include "deviceinfo/udisklistener.h"
#include "tag/tagmanager.h"

#include "dfileservices.h"
#include "dthumbnailprovider.h"
#include "dfileiconprovider.h"
#include "dmimedatabase.h"
#include "dabstractfilewatcher.h"
#include "dstorageinfo.h"
#include "shutil/danythingmonitorfilter.h"
#include "dfmapplication.h"

#ifdef SW_LABEL
#include "sw_label/filemanagerlibrary.h"
#endif

#include <QDateTime>
#include <QDir>
#include <QPainter>
#include <QApplication>
#include <QtConcurrent>
#include <qplatformdefs.h>
#include <ddiskmanager.h>

#include <sys/stat.h>
#include <unistd.h>

DFM_USE_NAMESPACE

DGvfsFileInfoPrivate::DGvfsFileInfoPrivate(const DUrl &url, DGvfsFileInfo *qq, bool hasCache)
    : DFileInfoPrivate(url, qq, hasCache)
{
    gvfsMountFile = true;
}

DGvfsFileInfoPrivate::~DGvfsFileInfoPrivate()
{
}

DGvfsFileInfo::DGvfsFileInfo(const QString &filePath, bool hasCache)
    : DGvfsFileInfo(DUrl::fromLocalFile(filePath), hasCache)
{
}

DGvfsFileInfo::DGvfsFileInfo(const DUrl &fileUrl, bool hasCache)
    : DFileInfo(*new DGvfsFileInfoPrivate(fileUrl, this, hasCache))
{
    //fix bug 27828 打开挂载文件（有很多的文件夹和文件）在断网的情况下，滑动鼠标或者滚动鼠标滚轮时文管卡死，做缓存
    refreshCachesByStat();
    canRename();
    isWritable();
    isSymLink();
    mimeType();
    size();
    //fix bug 35448 【文件管理器】【5.1.2.2-1】【sp2】预览ftp路径下某个文件夹后，文管卡死,访问特殊系统文件卡死
    //ftp挂载的系统proc中的文件夹获取filesCount，调用QDir和调用QDirIterator时，是gvfs文件的权限变成？？？
    //所以ftp和smb挂载点没有了，显示文件已被删除
//        filesCount();
}

DGvfsFileInfo::DGvfsFileInfo(const DUrl &fileUrl, const QMimeType &mimetype, bool hasCache)
    : DFileInfo(*new DGvfsFileInfoPrivate(fileUrl, this, hasCache))
{
    Q_D(const DGvfsFileInfo);

    if (mimetype.isValid()) {
        d->mimeType = mimetype;
        d->mimeTypeMode = QMimeDatabase::MatchExtension;
    }
    //fix bug 27828 打开挂载文件（有很多的文件夹和文件）在断网的情况下，滑动鼠标或者滚动鼠标滚轮时文管卡死，做缓存
    refreshCachesByStat();
    canRename();
    isWritable();
    isSymLink();
    //必须在isWritable之后
    if (!mimetype.isValid()) {
        mimeType();
    }
    size();
    //fix bug 35448 【文件管理器】【5.1.2.2-1】【sp2】预览ftp路径下某个文件夹后，文管卡死,访问特殊系统文件卡死
    //ftp挂载的系统proc中的文件夹获取filesCount，调用QDir和调用QDirIterator时，是gvfs文件的权限变成？？？
    //所以ftp和smb挂载点没有了，显示文件已被删除
}

DGvfsFileInfo::DGvfsFileInfo(const QFileInfo &fileInfo, bool hasCache)
    : DGvfsFileInfo(DUrl::fromLocalFile(fileInfo.absoluteFilePath()), hasCache)
{

}

DGvfsFileInfo::DGvfsFileInfo(const QFileInfo &fileInfo, const QMimeType &mimetype, bool hasCache)
    : DGvfsFileInfo(DUrl::fromLocalFile(fileInfo.absoluteFilePath()), mimetype, hasCache)
{

}

DGvfsFileInfo::~DGvfsFileInfo()
{

}

bool DGvfsFileInfo::exists() const
{
    Q_D(const DGvfsFileInfo);

    if (d->cacheFileExists < 0)
        d->cacheFileExists = d->fileInfo.exists() || d->fileInfo.isSymLink();

    if (d->cacheFileExists >= 0)
        return d->cacheFileExists;

    return d->fileInfo.exists() || d->fileInfo.isSymLink();
}

bool DGvfsFileInfo::canRename() const
{
    if (systemPathManager->isSystemPath(absoluteFilePath()))
        return false;
    Q_D(const DGvfsFileInfo);
    //fix bug 27828 修改gvfsMountFile属性时，也要缓存属性，才不会造成ftp和smb卡死
    if (d->cacheCanRename < 0) {
        d->cacheCanRename = DFileInfo::fileIsWritable(d->fileInfo.absolutePath(), static_cast<uint>(inode()));
    }

    if (d->cacheCanRename >= 0)
        return d->cacheCanRename;

    return fileIsWritable(d->fileInfo.absolutePath(), d->fileInfo.ownerId());
}

bool DGvfsFileInfo::isWritable() const
{
    if (isPrivate())
        return false;

    Q_D(const DGvfsFileInfo);
    //fix bug 27828 打开挂载文件（有很多的文件夹和文件）在断网的情况下，滑动鼠标或者滚动鼠标滚轮时文管卡死，做缓存
    if (d->cacheCanWrite < 0) {
        struct stat statinfo;
        int filestat = stat(d->fileInfo.absoluteFilePath().toStdString().c_str(), &statinfo);
        if (filestat == 0) {
            d->cacheCanWrite = access(d->fileInfo.absoluteFilePath().toStdString().c_str(), W_OK) == 0 ? 1 : 0;
        }
    }
    if ( d->cacheCanWrite >= 0) {
        return d->cacheCanWrite;
    }
    return d->fileInfo.isWritable();
}

bool DGvfsFileInfo::makeAbsolute()
{
    Q_D(DGvfsFileInfo);

    return d->fileInfo.makeAbsolute();
}

bool DGvfsFileInfo::isSymLink() const
{
    Q_D(const DGvfsFileInfo);
    //fix bug 27828 修改gvfsMountFile属性时，也要缓存属性，才不会造成ftp和smb卡死
    if (d->cacheIsSymLink < 0) {
        d->cacheIsSymLink = d->fileInfo.isSymLink();
    }

    if (d->cacheIsSymLink >= 0) {
        return d->cacheIsSymLink != 0;
    }

    return d->fileInfo.isSymLink();
}

bool DGvfsFileInfo::isDir() const
{
    Q_D(const DGvfsFileInfo);

    if (d->cacheIsDir > -1) {
        return d->cacheIsDir;
    }

    const_cast<DGvfsFileInfo *>(this)->refreshCachesByStat();

    if (d->cacheIsDir > -1) {
        return d->cacheIsDir;
    }

    return d->fileInfo.isDir();
}

QDateTime DGvfsFileInfo::lastRead() const
{
    Q_D(const DGvfsFileInfo);
    if (d->cacheReadTime > -1) {
        return QDateTime::fromTime_t(static_cast<uint>(d->cacheReadTime));
    }

    const_cast<DGvfsFileInfo *>(this)->refreshCachesByStat();

    if (d->cacheReadTime > -1) {
        return QDateTime::fromTime_t(static_cast<uint>(d->cacheReadTime));
    }

    return d->fileInfo.lastRead();
}

QDateTime DGvfsFileInfo::lastModified() const
{
    Q_D(const DGvfsFileInfo);
    if (d->cacheModifyTime > -1) {
        return QDateTime::fromTime_t(static_cast<uint>(d->cacheModifyTime));
    }

    const_cast<DGvfsFileInfo *>(this)->refreshCachesByStat();

    if (d->cacheModifyTime > -1) {
        return QDateTime::fromTime_t(static_cast<uint>(d->cacheModifyTime));
    }

    return d->fileInfo.lastModified();
}


uint DGvfsFileInfo::ownerId() const
{
    Q_D(const DGvfsFileInfo);

    if (-1 != d->ownerid) {
        return static_cast<uint>(d->ownerid);
    }

    return d->fileInfo.ownerId();
}


qint64 DGvfsFileInfo::size() const
{
    Q_D(const DGvfsFileInfo);

    if (d->cacheFileSize < 0) {
        d->cacheFileSize = d->fileInfo.size();
    }

    if (d->cacheFileSize >= 0) {
        return d->cacheFileSize;
    }

    return d->fileInfo.size();
}

int DGvfsFileInfo::filesCount() const
{
    Q_D(const DGvfsFileInfo);

    if (d->cacheFileCount < 0) {
        if (isDir())
            d->cacheFileCount = FileUtils::filesCount(absoluteFilePath());
        else
            return -1;
    }
    return d->cacheFileCount;
}

QMimeType DGvfsFileInfo::mimeType(QMimeDatabase::MatchMode mode) const
{
    Q_D(const DGvfsFileInfo);

    if (!d->mimeType.isValid() || d->mimeTypeMode != mode) {
        //优化是苹果的就用新的minetype
        DUrl url = fileUrl();
        //这里必须先缓存inod
        if (d->inode > 0) {
            QString inode = fileUrl().path() + QString::number(d->inode);
            d->mimeType = DFileInfo::mimeType(fileUrl().path(), QMimeDatabase::MatchExtension, inode);
        }
        else {
            d->mimeType = DFileInfo::mimeType(fileUrl().path(), mode);
        }
        d->mimeTypeMode = mode;
    }

    return d->mimeType;
}

void DGvfsFileInfo::refresh(const bool isForce)
{
    if (!isForce) {
        return;
    }
    Q_D(DGvfsFileInfo);
    DFileInfo::refresh(isForce);

    //fix bug 27828 打开挂载文件（有很多的文件夹和文件）在断网的情况下，滑动鼠标或者滚动鼠标滚轮时文管卡死，做缓存
    d->cacheCanWrite = -1;
    d->cacheCanRename = -1;
    d->cacheIsSymLink = -1;
    //fix bug 35831 cacheFileExists也需要清空 否则文件存在会被误判
    d->cacheFileExists = -1;
    d->ownerid = -1;
    d->cacheIsDir = -1;
    quint64 oldinod = d->inode;
    refreshCachesByStat();

    canRename();
    isWritable();
    isSymLink();
    if (!(oldinod > 0 && oldinod == d->inode)){
        mimeType();
    }
    size();
    //fix bug 35448 【文件管理器】【5.1.2.2-1】【sp2】预览ftp路径下某个文件夹后，文管卡死,访问特殊系统文件卡死
    //ftp挂载的系统proc中的文件夹获取filesCount，调用QDir和调用QDirIterator时，是gvfs文件的权限变成？？？
    //所以ftp和smb挂载点没有了，显示文件已被删除
}


void DGvfsFileInfo::makeToActive()
{
    Q_D(DGvfsFileInfo);

    d->fileInfo.refresh();

    DAbstractFileInfo::makeToActive();
}

quint64 DGvfsFileInfo::inode() const
{
    Q_D(const DGvfsFileInfo);
    return d->inode;
}
//用stat刷新所有的缓存
void DGvfsFileInfo::refreshCachesByStat()
{
    Q_D(DGvfsFileInfo);

    struct stat statinfo;
    int filestat = stat(d->fileInfo.absoluteFilePath().toStdString().c_str(), &statinfo);
    if (filestat != 0) {
        d->ownerid = static_cast<int>(getuid());
        return;
    }
    d->cacheCanWrite  = access(d->fileInfo.absoluteFilePath().toStdString().c_str(), W_OK) == 0 ? 1 : 0;
    d->cacheIsSymLink = S_ISLNK(statinfo.st_mode);
    d->cacheFileSize = statinfo.st_size;
    d->inode = statinfo.st_ino;
    d->ownerid = static_cast<int>(statinfo.st_uid);
    d->cacheIsDir = S_ISDIR(statinfo.st_mode);
    d->cacheModifyTime = statinfo.st_mtim.tv_sec;
    d->cacheReadTime = statinfo.st_atim.tv_sec;
}

DGvfsFileInfo::DGvfsFileInfo(DGvfsFileInfoPrivate &dd)
    : DFileInfo(dd)
{

}
