/*
 * Copyright (C) 2021 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
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
#include "base/standardpaths.h"
#include "base/schemefactory.h"
#include "utils/fileutils.h"
#include "utils/systempathutil.h"
#include "utils/chinese2pinyin.h"
#include "dfm-base/utils/dthumbnailprovider.h"
#include "dfm-base/file/local/localfileiconprovider.h"
#include "dfm-base/utils/sysinfoutils.h"

#include <dfm-io/dfmio_utils.h>
#include <dfm-io/local/dlocalfileinfo.h>

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

#define REQUEST_THUMBNAIL_DEALY 500

/*!
 * \class LocalFileInfo 本地文件信息类
 * \brief 内部实现本地文件的fileinfo，对应url的scheme是file://
 */
DFMBASE_BEGIN_NAMESPACE

LocalFileInfo::LocalFileInfo(const QUrl &url)
    : AbstractFileInfo(url, new LocalFileInfoPrivate(this))
{
    d = static_cast<LocalFileInfoPrivate *>(dptr.data());
    init(url);
}

LocalFileInfo::~LocalFileInfo()
{
    d = nullptr;
}

LocalFileInfo &LocalFileInfo::operator=(const LocalFileInfo &info)
{
    QReadLocker locker(&d->lock);
    AbstractFileInfo::operator=(info);
    d->url = info.d->url;
    d->dfmFileInfo = info.d->dfmFileInfo;
    d->inode = info.d->inode;
    return *this;
}
/*!
 * \brief == 重载操作符==
 *
 * \param const DAbstractFileInfo & DAbstractFileInfo实例对象的引用
 *
 * \return bool 传入的DAbstractFileInfo实例对象和自己是否相等
 */
bool LocalFileInfo::operator==(const LocalFileInfo &fileinfo) const
{
    return d->dfmFileInfo == fileinfo.d->dfmFileInfo && d->url == fileinfo.d->url;
}
/*!
 * \brief != 重载操作符!=
 *
 * \param const LocalFileInfo & LocalFileInfo实例对象的引用
 *
 * \return bool 传入的LocalFileInfo实例对象和自己是否不相等
 */
bool LocalFileInfo::operator!=(const LocalFileInfo &fileinfo) const
{
    return !(operator==(fileinfo));
}

/*!
 * \brief setFile 设置文件的File，跟新当前的fileinfo
 *
 * \param const QSharedPointer<DFMIO::DFileInfo> &file 新文件的dfm-io的fileinfo
 *
 * \return
 */
void LocalFileInfo::setFile(const QUrl &url)
{
    QWriteLocker locker(&d->lock);
    d->url = url;
    init(url);
}
/*!
 * \brief exists 文件是否存在
 *
 * \param
 *
 * \return 返回文件是否存在
 */
bool LocalFileInfo::exists() const
{
    QReadLocker locker(&d->lock);
    bool exists = false;

    if (d->dfmFileInfo) {
        exists = d->dfmFileInfo->exists();
    } else {
        exists = QFileInfo::exists(d->url.path());
    }

    return exists;
}
/*!
 * \brief refresh 更新文件信息，清理掉缓存的所有的文件信息
 *
 * \param
 *
 * \return
 */
void LocalFileInfo::refresh()
{
    QWriteLocker locker(&d->lock);
    d->attributes.clear();
    d->attributesExtend.clear();
    d->clearIcon();
    d->dfmFileInfo->clearCache();
    d->dfmFileInfo->flush();
}

void LocalFileInfo::refresh(DFileInfo::AttributeID id, const QVariant &value)
{
    QWriteLocker locker(&d->lock);
    if (value.isValid())
        d->dfmFileInfo->setAttribute(id, value);
    if (d->attributes.contains(id))
        d->attributes.remove(id);
}
/*!
 * \brief filePath 获取文件的绝对路径，含文件的名称，相当于文件的全路径
 *
 * url = file:///tmp/archive.tar.gz
 *
 * filePath = /tmp/archive.tar.gz
 *
 * \param
 *
 * \return
 */
QString LocalFileInfo::filePath() const
{
    //文件的路径
    QReadLocker locker(&d->lock);
    QString filePath;

    if (d->attributes.count(DFileInfo::AttributeID::StandardFilePath) == 0) {
        bool success = false;
        if (d->dfmFileInfo) {
            filePath = d->dfmFileInfo->attribute(DFileInfo::AttributeID::StandardFilePath, &success).toString();
        }
        if (!success)
            filePath = QFileInfo(d->url.path()).filePath();
        d->attributes.insert(DFileInfo::AttributeID::StandardFilePath, filePath);
    } else {
        filePath = d->attributes.value(DFileInfo::AttributeID::StandardFilePath).toString();
    }

    return filePath;
}

/*!
 * \brief absoluteFilePath 获取文件的绝对路径，含文件的名称，相当于文件的全路径，事例如下：
 *
 * url = file:///tmp/archive.tar.gz
 *
 * absoluteFilePath = /tmp/archive.tar.gz
 *
 * \param
 *
 * \return
 */
QString LocalFileInfo::absoluteFilePath() const
{
    return filePath();
}
/*!
 * \brief fileName 文件名称，全名称
 *
 * url = file:///tmp/archive.tar.gz
 *
 * fileName = archive.tar.gz
 *
 * \param
 *
 * \return
 */
QString LocalFileInfo::fileName() const
{
    QReadLocker locker(&d->lock);
    QString fileName;

    if (d->attributes.count(DFileInfo::AttributeID::StandardName) == 0) {
        bool success = false;
        if (d->dfmFileInfo) {
            fileName = d->dfmFileInfo->attribute(DFileInfo::AttributeID::StandardName, &success).toString();

            // trans "/" to "smb-share:server=xxx,share=xxx"
            if (fileName == R"(/)" && FileUtils::isGvfsFile(d->url)) {
                fileName = d->dfmFileInfo->attribute(DFileInfo::AttributeID::IdFilesystem, &success).toString();
            }
        }
        if (!success)
            fileName = QFileInfo(d->url.path()).fileName();

        d->attributes.insert(DFileInfo::AttributeID::StandardName, fileName);
    } else {
        fileName = d->attributes.value(DFileInfo::AttributeID::StandardName).toString();
    }

    return fileName;
}
/*!
 * \brief baseName 文件的基本名称
 *
 * url = file:///tmp/archive.tar.gz
 *
 * baseName = archive
 *
 * \param
 *
 * \return
 */
QString LocalFileInfo::baseName() const
{
    QReadLocker locker(&d->lock);
    QString baseName;

    if (d->attributes.count(DFileInfo::AttributeID::StandardBaseName) == 0) {
        bool success = false;
        if (d->dfmFileInfo) {
            baseName = d->dfmFileInfo->attribute(DFileInfo::AttributeID::StandardBaseName, &success).toString();
        }
        if (!success)
            baseName = QFileInfo(d->url.path()).baseName();

        d->attributes.insert(DFileInfo::AttributeID::StandardBaseName, baseName);
    } else {
        baseName = d->attributes.value(DFileInfo::AttributeID::StandardBaseName).toString();
    }

    return baseName;
}
/*!
 * \brief completeBaseName 文件的完整基本名称
 *
 * url = file:///tmp/archive.tar.gz
 *
 * completeBaseName = archive.tar
 *
 * \param
 *
 * \return
 */
QString LocalFileInfo::completeBaseName() const
{
    QReadLocker locker(&d->lock);
    QString completeBaseName;

    if (d->attributes.count(DFileInfo::AttributeID::StandardCompleteBaseName) == 0) {
        bool success = false;
        if (d->dfmFileInfo) {
            completeBaseName = d->dfmFileInfo->attribute(DFileInfo::AttributeID::StandardCompleteBaseName, &success).toString();
        }
        if (!success)
            completeBaseName = QFileInfo(d->url.path()).completeBaseName();

        d->attributes.insert(DFileInfo::AttributeID::StandardCompleteBaseName, completeBaseName);
    } else {
        completeBaseName = d->attributes.value(DFileInfo::AttributeID::StandardCompleteBaseName).toString();
    }

    return completeBaseName;
}
/*!
 * \brief suffix 文件的suffix
 *
 * url = file:///tmp/archive.tar.gz
 *
 * suffix = gz
 *
 * \param
 *
 * \return
 */
QString LocalFileInfo::suffix() const
{
    QReadLocker locker(&d->lock);
    QString suffix;

    if (d->attributes.count(DFileInfo::AttributeID::StandardSuffix) == 0) {
        bool success = false;
        if (d->dfmFileInfo) {
            suffix = d->dfmFileInfo->attribute(DFileInfo::AttributeID::StandardSuffix, &success).toString();
        }
        if (!success)
            suffix = QFileInfo(d->url.path()).suffix();

        d->attributes.insert(DFileInfo::AttributeID::StandardSuffix, suffix);
    } else {
        suffix = d->attributes.value(DFileInfo::AttributeID::StandardSuffix).toString();
    }

    return suffix;
}
/*!
 * \brief suffix 文件的完整suffix
 *
 * url = file:///tmp/archive.tar.gz
 *
 * suffix = tar.gz
 *
 * \param
 *
 * \return
 */
QString LocalFileInfo::completeSuffix()
{
    QReadLocker locker(&d->lock);
    QString completeSuffix;

    if (d->attributes.count(DFileInfo::AttributeID::StandardCompleteSuffix) == 0) {
        bool success = false;
        if (d->dfmFileInfo) {
            completeSuffix = d->dfmFileInfo->attribute(DFileInfo::AttributeID::StandardCompleteSuffix, &success).toString();
        }
        if (!success)
            completeSuffix = QFileInfo(d->url.path()).completeSuffix();

        d->attributes.insert(DFileInfo::AttributeID::StandardCompleteSuffix, completeSuffix);
    } else {
        completeSuffix = d->attributes.value(DFileInfo::AttributeID::StandardCompleteSuffix).toString();
    }

    return completeSuffix;
}
/*!
 * \brief path 获取文件路径，不包含文件的名称，相当于是父目录
 *
 * url = file:///tmp/archive.tar.gz
 *
 * path = /tmp
 *
 * \param
 *
 * \return
 */
QString LocalFileInfo::path() const
{
    QReadLocker locker(&d->lock);
    QString path;

    if (d->attributes.count(DFileInfo::AttributeID::StandardParentPath) == 0) {
        bool success = false;
        if (d->dfmFileInfo) {
            path = d->dfmFileInfo->attribute(DFileInfo::AttributeID::StandardParentPath, &success).toString();
        }
        if (!success)
            path = QFileInfo(d->url.path()).path();

        d->attributes.insert(DFileInfo::AttributeID::StandardParentPath, path);
    } else {
        path = d->attributes.value(DFileInfo::AttributeID::StandardParentPath).toString();
    }

    return path;
}
/*!
 * \brief path 获取文件路径，不包含文件的名称，相当于是父目录
 *
 * url = file:///tmp/archive.tar.gz
 *
 * absolutePath = /tmp
 *
 * \param
 *
 * \return
 */
QString LocalFileInfo::absolutePath() const
{
    QReadLocker locker(&d->lock);
    QString path;

    if (d->attributes.count(DFileInfo::AttributeID::StandardParentPath) == 0) {
        bool success = false;
        if (d->dfmFileInfo) {
            path = d->dfmFileInfo->attribute(DFileInfo::AttributeID::StandardParentPath, &success).toString();
        }
        if (!success)
            path = QFileInfo(d->url.path()).absolutePath();

        d->attributes.insert(DFileInfo::AttributeID::StandardParentPath, path);
    } else {
        path = d->attributes.value(DFileInfo::AttributeID::StandardParentPath).toString();
    }

    return path;
}
/*!
 * \brief canonicalPath 获取文件canonical路径，包含文件的名称，相当于文件的全路径
 *
 * url = file:///tmp/archive.tar.gz
 *
 * canonicalPath = /tmp/archive.tar.gz
 *
 * \param
 *
 * \return QString 返回没有符号链接或冗余“.”或“..”元素的绝对路径
 */
QString LocalFileInfo::canonicalPath() const
{
    return filePath();
}
/*!
 * \brief dir 获取文件的父母目录的QDir
 *
 * Returns the path of the object's parent directory as a QDir object.
 *
 * url = file:///tmp/archive.tar.gz
 *
 * dirpath = /tmp
 *
 * \param
 *
 * \return QDir 父母目录的QDir实例对象
 */
QDir LocalFileInfo::dir() const
{
    return QDir(path());
}
/*!
 * \brief absoluteDir 获取文件的父母目录的QDir
 *
 * Returns the file's absolute path as a QDir object.
 *
 * url = file:///tmp/archive.tar.gz
 *
 * absolute path = /tmp
 *
 * \param
 *
 * \return QDir 父母目录的QDir实例对象
 */
QDir LocalFileInfo::absoluteDir() const
{
    return dir();
}
/*!
 * \brief url 获取文件的url，这里的url是转换后的
 *
 * \param
 *
 * \return QUrl 返回真实路径转换的url
 */
QUrl LocalFileInfo::url() const
{
    QReadLocker locker(&d->lock);
    return d->url;
}

bool LocalFileInfo::canRename() const
{
    if (SystemPathUtil::instance()->isSystemPath(absoluteFilePath()))
        return false;

    QReadLocker locker(&d->lock);
    bool canRename = false;

    if (d->attributes.count(DFileInfo::AttributeID::AccessCanRename) == 0) {
        canRename = SysInfoUtils::isRootUser();
        if (!canRename) {
            int result = access(d->url.path().toLocal8Bit().data(), W_OK);
            canRename = result == 0;
        }
        if (!canRename) {
            if (d->dfmFileInfo)
                canRename = d->dfmFileInfo->attribute(DFileInfo::AttributeID::AccessCanRename, nullptr).toBool();
        }
        d->attributes.insert(DFileInfo::AttributeID::AccessCanRename, canRename);
    } else {
        canRename = d->attributes.value(DFileInfo::AttributeID::AccessCanRename).toBool();
    }

    return canRename;
}

bool LocalFileInfo::canTag() const
{
    // todo lanxs
    return false;
}
/*!
 * \brief isReadable 获取文件是否可读
 *
 * Returns the file can Read
 *
 * url = file:///tmp/archive.tar.gz
 *
 * \param
 *
 * \return bool 返回文件是否可读
 */
bool LocalFileInfo::isReadable() const
{
    // todo lanxs
    // check file isprivate

    QReadLocker locker(&d->lock);
    bool isReadable = false;

    if (d->attributes.count(DFileInfo::AttributeID::AccessCanRead) == 0) {
        bool success = false;
        if (d->dfmFileInfo) {
            isReadable = d->dfmFileInfo->attribute(DFileInfo::AttributeID::AccessCanRead, &success).toBool();
        }
        if (!success)
            isReadable = QFileInfo(d->url.path()).isReadable();

        d->attributes.insert(DFileInfo::AttributeID::AccessCanRead, isReadable);
    } else {
        isReadable = d->attributes.value(DFileInfo::AttributeID::AccessCanRead).toBool();
    }

    return isReadable;
}
/*!
 * \brief isWritable 获取文件是否可写
 *
 * Returns the file can write
 *
 * url = file:///tmp/archive.tar.gz
 *
 * \param
 *
 * \return bool 返回文件是否可写
 */
bool LocalFileInfo::isWritable() const
{
    QReadLocker locker(&d->lock);
    bool isWritable = false;

    if (d->attributes.count(DFileInfo::AttributeID::AccessCanWrite) == 0) {
        bool success = false;
        if (d->dfmFileInfo)
            isWritable = d->dfmFileInfo->attribute(DFileInfo::AttributeID::AccessCanWrite, &success).toBool();

        if (!success)
            isWritable = QFileInfo(d->url.path()).isWritable();

        d->attributes.insert(DFileInfo::AttributeID::AccessCanWrite, isWritable);
    } else {
        isWritable = d->attributes.value(DFileInfo::AttributeID::AccessCanWrite).toBool();
    }

    return isWritable;
}
/*!
 * \brief isExecutable 获取文件是否可执行
 *
 * \param
 *
 * \return bool 返回文件是否可执行
 */
bool LocalFileInfo::isExecutable() const
{
    QReadLocker locker(&d->lock);
    bool isExecutable = false;

    if (d->attributes.count(DFileInfo::AttributeID::AccessCanExecute) == 0) {
        bool success = false;
        if (d->dfmFileInfo) {
            isExecutable = d->dfmFileInfo->attribute(DFileInfo::AttributeID::AccessCanExecute, &success).toBool();
        }
        if (!success)
            isExecutable = QFileInfo(d->url.path()).isExecutable();

        d->attributes.insert(DFileInfo::AttributeID::AccessCanExecute, isExecutable);
    } else {
        isExecutable = d->attributes.value(DFileInfo::AttributeID::AccessCanExecute).toBool();
    }

    return isExecutable;
}
/*!
 * \brief isHidden 获取文件是否是隐藏
 *
 * \param
 *
 * \return bool 返回文件是否隐藏
 */
bool LocalFileInfo::isHidden() const
{
    QReadLocker locker(&d->lock);
    bool isHidden = false;

    if (d->attributes.count(DFileInfo::AttributeID::StandardIsHidden) == 0) {
        if (d->dfmFileInfo) {
            isHidden = d->dfmFileInfo->attribute(DFileInfo::AttributeID::StandardIsHidden, nullptr).toBool();
        }
        d->attributes.insert(DFileInfo::AttributeID::StandardIsHidden, isHidden);
    } else {
        isHidden = d->attributes.value(DFileInfo::AttributeID::StandardIsHidden).toBool();
    }

    return isHidden;
}
/*!
 * \brief isFile 获取文件是否是文件
 *
 * Returns true if this object points to a file or to a symbolic link to a file.
 *
 * Returns false if the object points to something which isn't a file,
 *
 * such as a directory.
 *
 * \param
 *
 * \return bool 返回文件是否文件
 */
bool LocalFileInfo::isFile() const
{
    QReadLocker locker(&d->lock);
    bool isFile = false;

    if (d->attributes.count(DFileInfo::AttributeID::StandardIsFile) == 0) {
        bool success = false;
        if (d->dfmFileInfo) {
            isFile = d->dfmFileInfo->attribute(DFileInfo::AttributeID::StandardIsFile, &success).toBool();
        }
        if (!success)
            isFile = QFileInfo(d->url.path()).isFile();
        d->attributes.insert(DFileInfo::AttributeID::StandardIsFile, isFile);
    } else {
        isFile = d->attributes.value(DFileInfo::AttributeID::StandardIsFile).toBool();
    }

    return isFile;
}
/*!
 * \brief isDir 获取文件是否是目录
 *
 * Returns true if this object points to a directory or to a symbolic link to a directory;
 *
 * otherwise returns false.
 *
 * \param
 *
 * \return bool 返回文件是否目录
 */
bool LocalFileInfo::isDir() const
{
    QReadLocker locker(&d->lock);
    bool isDir = false;

    if (d->attributes.count(DFileInfo::AttributeID::StandardIsDir) == 0) {
        bool success = false;
        if (d->dfmFileInfo) {
            isDir = d->dfmFileInfo->attribute(DFileInfo::AttributeID::StandardIsDir, &success).toBool();
        }
        if (!success)
            isDir = QFileInfo(d->url.path()).isDir();
        d->attributes.insert(DFileInfo::AttributeID::StandardIsDir, isDir);
    } else {
        isDir = d->attributes.value(DFileInfo::AttributeID::StandardIsDir).toBool();
    }
    return isDir;
}
/*!
 * \brief isSymLink 获取文件是否是链接文件
 *
 * Returns true if this object points to a symbolic link;
 *
 * otherwise returns false.Symbolic links exist on Unix (including macOS and iOS)
 *
 * and Windows and are typically created by the ln -s or mklink commands, respectively.
 *
 * Opening a symbolic link effectively opens the link's target.
 *
 * In addition, true will be returned for shortcuts (*.lnk files) on Windows.
 *
 * Opening those will open the .lnk file itself.
 *
 * \param
 *
 * \return bool 返回文件是否是链接文件
 */
bool LocalFileInfo::isSymLink() const
{
    QReadLocker locker(&d->lock);
    bool isSymLink = false;

    if (d->attributes.count(DFileInfo::AttributeID::StandardIsSymlink) == 0) {
        bool success = false;
        if (d->dfmFileInfo) {
            isSymLink = d->dfmFileInfo->attribute(DFileInfo::AttributeID::StandardIsSymlink, &success).toBool();
        }
        if (!success)
            isSymLink = QFileInfo(d->url.path()).isSymLink();
        d->attributes.insert(DFileInfo::AttributeID::StandardIsSymlink, isSymLink);
    } else {
        isSymLink = d->attributes.value(DFileInfo::AttributeID::StandardIsSymlink).toBool();
    }
    return isSymLink;
}
/*!
 * \brief isRoot 获取文件是否是根目录
 *
 * Returns true if the object points to a directory or to a symbolic link to a directory,
 *
 * and that directory is the root directory; otherwise returns false.
 *
 * \param
 *
 * \return bool 返回文件是否是根目录
 */
bool LocalFileInfo::isRoot() const
{
    return filePath() == "/";
}
/*!
 * \brief isBundle 获取文件是否是二进制文件
 *
 * Returns true if this object points to a bundle or to a symbolic
 *
 * link to a bundle on macOS and iOS; otherwise returns false.
 *
 * \param
 *
 * \return bool 返回文件是否是二进制文件
 */
bool LocalFileInfo::isBundle() const
{
    QReadLocker locker(&d->lock);
    bool isBundle = QFileInfo(d->url.path()).isBundle();
    return isBundle;
}

bool LocalFileInfo::isShared() const
{
    // todo lanxs
    return false;
}
/*!
 * \brief isBundle 获取文件的链接目标文件
 *
 * Returns the absolute path to the file or directory a symbolic link points to,
 *
 * or an empty string if the object isn't a symbolic link.
 *
 * This name may not represent an existing file; it is only a string.
 *
 * QFileInfo::exists() returns true if the symlink points to an existing file.
 *
 * \param
 *
 * \return QString 链接目标文件的路径
 */
QString LocalFileInfo::symLinkTarget() const
{
    QReadLocker locker(&d->lock);
    QString symLinkTarget;

    if (d->attributes.count(DFileInfo::AttributeID::StandardSymlinkTarget) == 0) {
        bool success = false;
        if (d->dfmFileInfo) {
            symLinkTarget = d->dfmFileInfo->attribute(DFileInfo::AttributeID::StandardSymlinkTarget, &success).toString();
        }
        if (!success)
            symLinkTarget = QFileInfo(d->url.path()).symLinkTarget();
        d->attributes.insert(DFileInfo::AttributeID::StandardSymlinkTarget, symLinkTarget);
    } else {
        symLinkTarget = d->attributes.value(DFileInfo::AttributeID::StandardSymlinkTarget).toString();
    }

    return symLinkTarget;
}
/*!
 * \brief owner 获取文件的拥有者
 *
 * Returns the owner of the file. On systems where files do not have owners,
 *
 * or if an error occurs, an empty string is returned.
 *
 * This function can be time consuming under Unix (in the order of milliseconds).
 *
 * \param
 *
 * \return QString 文件的拥有者
 */
QString LocalFileInfo::owner() const
{
    QReadLocker locker(&d->lock);
    QString owner;

    if (d->attributes.count(DFileInfo::AttributeID::OwnerUser) == 0) {
        bool success = false;
        if (d->dfmFileInfo) {
            owner = d->dfmFileInfo->attribute(DFileInfo::AttributeID::OwnerUser, &success).toString();
        }
        if (!success)
            owner = QFileInfo(d->url.path()).owner();
        d->attributes.insert(DFileInfo::AttributeID::OwnerUser, owner);
    } else {
        owner = d->attributes.value(DFileInfo::AttributeID::OwnerUser).toString();
    }
    return owner;
}
/*!
 * \brief ownerId 获取文件的拥有者ID
 *
 * Returns the id of the owner of the file.
 * On Windows and on systems where files do not have owners this function returns ((uint) -2).
 *
 * \param
 *
 * \return uint 文件的拥有者ID
 */
uint LocalFileInfo::ownerId() const
{
    QReadLocker locker(&d->lock);
    uint ownerId = uint(-2);

    if (d->attributes.count(DFileInfo::AttributeID::UnixUID) == 0) {
        bool success = false;
        if (d->dfmFileInfo) {
            ownerId = d->dfmFileInfo->attribute(DFileInfo::AttributeID::UnixUID, &success).toUInt();
        }
        if (!success)
            ownerId = QFileInfo(d->url.path()).ownerId();
        d->attributes.insert(DFileInfo::AttributeID::UnixUID, ownerId);
    } else {
        ownerId = d->attributes.value(DFileInfo::AttributeID::UnixUID).toUInt();
    }
    return ownerId;
}
/*!
 * \brief group 获取文件所属组
 *
 * Returns the group of the file.
 *
 * This function can be time consuming under Unix (in the order of milliseconds).
 *
 * \param
 *
 * \return QString 文件所属组
 */
QString LocalFileInfo::group() const
{
    QReadLocker locker(&d->lock);
    QString group;

    if (d->attributes.count(DFileInfo::AttributeID::OwnerGroup) == 0) {
        bool success = false;
        if (d->dfmFileInfo) {
            group = d->dfmFileInfo->attribute(DFileInfo::AttributeID::OwnerGroup, &success).toString();
        }
        if (!success)
            group = QFileInfo(d->url.path()).group();
        d->attributes.insert(DFileInfo::AttributeID::OwnerGroup, group);
    } else {
        group = d->attributes.value(DFileInfo::AttributeID::OwnerGroup).toString();
    }
    return group;
}
/*!
 * \brief groupId 获取文件所属组的ID
 *
 * Returns the id of the group the file belongs to.
 *
 * \param
 *
 * \return uint 文件所属组ID
 */
uint LocalFileInfo::groupId() const
{
    QReadLocker locker(&d->lock);
    uint groupId = 0;

    if (d->attributes.count(DFileInfo::AttributeID::OwnerGroup) == 0) {
        bool success = false;
        if (d->dfmFileInfo) {
            groupId = d->dfmFileInfo->attribute(DFileInfo::AttributeID::OwnerGroup, &success).toUInt();
        }
        if (!success)
            groupId = QFileInfo(d->url.path()).groupId();
        d->attributes.insert(DFileInfo::AttributeID::OwnerGroup, groupId);
    } else {
        groupId = d->attributes.value(DFileInfo::AttributeID::OwnerGroup).toUInt();
    }
    return groupId;
}
/*!
 * \brief permission 判断文件是否有传入的权限
 *
 * Tests for file permissions. The permissions argument can be several flags
 *
 * of type QFile::Permissions OR-ed together to check for permission combinations.
 *
 * On systems where files do not have permissions this function always returns true.
 *
 * \param QFile::Permissions permissions 文件的权限
 *
 * \return bool 是否有传入的权限
 */
bool LocalFileInfo::permission(QFileDevice::Permissions permissions) const
{
    return this->permissions() & permissions;
}
/*!
 * \brief permissions 获取文件的全部权限
 *
 * \param
 *
 * \return QFile::Permissions 文件的全部权限
 */
QFileDevice::Permissions LocalFileInfo::permissions() const
{
    QReadLocker locker(&d->lock);
    QFileDevice::Permissions ps;

    if (d->dfmFileInfo) {
        ps = static_cast<QFileDevice::Permissions>(static_cast<uint16_t>(d->dfmFileInfo->permissions()));
    }
    if (ps == 0x0000)
        ps = QFileInfo(d->url.path()).permissions();

    return ps;
}
/*!
 * \brief size 获取文件的大小
 *
 * Returns the file size in bytes.
 *
 * If the file does not exist or cannot be fetched, 0 is returned.
 *
 * \param
 *
 * \return qint64 文件的大小
 */
qint64 LocalFileInfo::size() const
{
    QReadLocker locker(&d->lock);
    qint64 size = 0;

    if (d->attributes.count(DFileInfo::AttributeID::StandardSize) == 0) {
        bool success = false;
        if (d->dfmFileInfo) {
            size = d->dfmFileInfo->attribute(DFileInfo::AttributeID::StandardSize, &success).value<qint64>();
        }
        if (!success)
            size = QFileInfo(d->url.path()).size();
        d->attributes.insert(DFileInfo::AttributeID::StandardSize, size);
    } else {
        size = d->attributes.value(DFileInfo::AttributeID::StandardSize).toLongLong();
    }
    return size;
}
/*!
 * \brief created 获取文件的创建时间
 *
 * Returns the date and time when the file was created,
 *
 * the time its metadata was last changed or the time of last modification,
 *
 * whichever one of the three is available (in that order).
 *
 * \param
 *
 * \return QDateTime 文件的创建时间的QDateTime实例
 */
QDateTime LocalFileInfo::created() const
{
    QReadLocker locker(&d->lock);
    QDateTime time;

    if (d->attributes.count(DFileInfo::AttributeID::TimeCreated) == 0) {
        bool success = false;
        if (d->dfmFileInfo) {
            uint64_t created = d->dfmFileInfo->attribute(DFileInfo::AttributeID::TimeCreated, &success).value<uint64_t>();
            if (success)
                time = QDateTime::fromTime_t(static_cast<uint>(created));
        }
        if (!success)
            time = QFileInfo(d->url.path()).created();
        d->attributes.insert(DFileInfo::AttributeID::TimeCreated, time);
    } else {
        time = d->attributes.value(DFileInfo::AttributeID::TimeCreated).toDateTime();
    }
    return time;
}
/*!
 * \brief birthTime 获取文件的创建时间
 *
 * Returns the date and time when the file was created / born.
 *
 * If the file birth time is not available, this function
 *
 * returns an invalid QDateTime.
 *
 * \param
 *
 * \return QDateTime 文件的创建时间的QDateTime实例
 */
QDateTime LocalFileInfo::birthTime() const
{
    return created();
}
/*!
 * \brief metadataChangeTime 获取文件的改变时间
 *
 * Returns the date and time when the file metadata was changed.
 *
 * A metadata change occurs when the file is created,
 *
 * but it also occurs whenever the user writes or sets
 *
 * inode information (for example, changing the file permissions).
 *
 * \param
 *
 * \return QDateTime 文件的改变时间的QDateTime实例
 */
QDateTime LocalFileInfo::metadataChangeTime() const
{
    QReadLocker locker(&d->lock);
    QDateTime time;

    if (d->attributes.count(DFileInfo::AttributeID::TimeChanged) == 0) {
        bool success = false;
        if (d->dfmFileInfo) {
            uint64_t data = d->dfmFileInfo->attribute(DFileInfo::AttributeID::TimeChanged, &success).value<uint64_t>();
            if (success)
                time = QDateTime::fromTime_t(static_cast<uint>(data));
        }
        if (!success) {
            time = QFileInfo(d->url.path()).metadataChangeTime();
        }
        d->attributes.insert(DFileInfo::AttributeID::TimeChanged, time);
    } else {
        time = d->attributes.value(DFileInfo::AttributeID::TimeChanged).toDateTime();
    }
    return time;
}
/*!
 * \brief lastModified 获取文件的最后修改时间
 *
 * \param
 *
 * \return QDateTime 文件的最后修改时间的QDateTime实例
 */
QDateTime LocalFileInfo::lastModified() const
{
    QReadLocker locker(&d->lock);
    QDateTime time;

    if (d->attributes.count(DFileInfo::AttributeID::TimeModified) == 0) {
        bool success = false;
        if (d->dfmFileInfo) {
            uint64_t data = d->dfmFileInfo->attribute(DFileInfo::AttributeID::TimeModified, &success).value<uint64_t>();
            if (success)
                time = QDateTime::fromTime_t(static_cast<uint>(data));
        }
        if (!success)
            time = QFileInfo(d->url.path()).lastModified();
        d->attributes.insert(DFileInfo::AttributeID::TimeModified, time);
    } else {
        time = d->attributes.value(DFileInfo::AttributeID::TimeModified).toDateTime();
    }
    return time;
}
/*!
 * \brief lastRead 获取文件的最后读取时间
 *
 * \param
 *
 * \return QDateTime 文件的最后读取时间的QDateTime实例
 */
QDateTime LocalFileInfo::lastRead() const
{
    QReadLocker locker(&d->lock);
    QDateTime time;

    if (d->attributes.count(DFileInfo::AttributeID::TimeAccess) == 0) {
        bool success = false;
        if (d->dfmFileInfo) {
            uint64_t data = d->dfmFileInfo->attribute(DFileInfo::AttributeID::TimeAccess, &success).value<uint64_t>();
            if (success)
                time = QDateTime::fromTime_t(static_cast<uint>(data));
        }
        if (!success)
            time = QFileInfo(d->url.path()).lastRead();
        d->attributes.insert(DFileInfo::AttributeID::TimeAccess, time);
    } else {
        time = d->attributes.value(DFileInfo::AttributeID::TimeAccess).toDateTime();
    }
    return time;
}
/*!
 * \brief fileTime 获取文件的事件通过传入的参数
 *
 * \param QFile::FileTime time 时间类型
 *
 * \return QDateTime 文件的不同时间类型的时间的QDateTime实例
 */
QDateTime LocalFileInfo::fileTime(QFileDevice::FileTime time) const
{
    if (time == QFileDevice::FileAccessTime) {
        return lastRead();
    } else if (time == QFileDevice::FileBirthTime) {
        return created();
    } else if (time == QFileDevice::FileMetadataChangeTime) {
        return metadataChangeTime();
    } else if (time == QFileDevice::FileModificationTime) {
        return lastModified();
    } else {
        return QDateTime();
    }
}
/*!
 * \brief isBlockDev 获取是否是块设备
 *
 * \return bool 是否是块设备
 */
bool LocalFileInfo::isBlockDev() const
{
    return fileType() == kBlockDevice;
}
/*!
 * \brief mountPath 获取挂载路径
 *
 * \return QString 挂载路径
 */
QString LocalFileInfo::mountPath() const
{
    if (isBlockDev())
        return DFMIO::DFMUtils::devicePathFromUrl(d->url);
    else
        return QString();
}
/*!
 * \brief isCharDev 获取是否是字符设备
 *
 * \return bool 是否是字符设备
 */
bool LocalFileInfo::isCharDev() const
{
    return fileType() == kCharDevice;
}
/*!
 * \brief isFifo 获取当前是否为管道文件
 *
 * \return bool 返回当前是否为管道文件
 */
bool LocalFileInfo::isFifo() const
{
    return fileType() == kFIFOFile;
}
/*!
 * \brief isSocket 获取当前是否为套接字文件
 *
 * \return bool 返回是否为套接字文件
 */
bool LocalFileInfo::isSocket() const
{
    return fileType() == kSocketFile;
}
/*!
 * \brief isRegular 获取当前是否是常规文件(与isFile一致)
 *
 * \return bool 返回是否是常规文件(与isFile一致)
 */
bool LocalFileInfo::isRegular() const
{
    return fileType() == kRegularFile;
}

/*!
 * \brief fileType 获取文件类型
 *
 * \return DMimeDatabase::FileType 文件设备类型
 */
LocalFileInfo::FileType LocalFileInfo::fileType() const
{
    QReadLocker locker(&d->lock);
    FileType fileType;
    if (d->fileType != MimeDatabase::FileType::kUnknown) {
        fileType = FileType(d->fileType);
        return fileType;
    }

    QString absoluteFilePath = filePath();
    if (absoluteFilePath.startsWith(StandardPaths::location(StandardPaths::kTrashFilesPath))
        && isSymLink()) {
        d->fileType = MimeDatabase::FileType::kRegularFile;
        fileType = FileType(d->fileType);
        return fileType;
    }

    // Cannot access statBuf.st_mode from the filesystem engine, so we have to stat again.
    // In addition we want to follow symlinks.
    const QByteArray &nativeFilePath = QFile::encodeName(absoluteFilePath);
    QT_STATBUF statBuffer;
    if (QT_STAT(nativeFilePath.constData(), &statBuffer) == 0) {
        if (S_ISDIR(statBuffer.st_mode))
            d->fileType = MimeDatabase::FileType::kDirectory;

        else if (S_ISCHR(statBuffer.st_mode))
            d->fileType = MimeDatabase::FileType::kCharDevice;

        else if (S_ISBLK(statBuffer.st_mode))
            d->fileType = MimeDatabase::FileType::kBlockDevice;

        else if (S_ISFIFO(statBuffer.st_mode))
            d->fileType = MimeDatabase::FileType::kFIFOFile;

        else if (S_ISSOCK(statBuffer.st_mode))
            d->fileType = MimeDatabase::FileType::kSocketFile;

        else if (S_ISREG(statBuffer.st_mode))
            d->fileType = MimeDatabase::FileType::kRegularFile;
    }

    fileType = FileType(d->fileType);
    return fileType;
}
/*!
 * \brief countChildFile 文件夹下子文件的个数，只统计下一层不递归
 *
 * \return int 子文件个数
 */
int LocalFileInfo::countChildFile() const
{
    if (isDir()) {
        QReadLocker locker(&d->lock);
        QDir dir(absoluteFilePath());
        QStringList entryList = dir.entryList(QDir::AllEntries | QDir::System
                                              | QDir::NoDotAndDotDot | QDir::Hidden);
        return entryList.size();
    }

    return -1;
}
/*!
 * \brief sizeFormat 格式化大小
 * \return QString 大小格式化后的大小
 */
QString LocalFileInfo::sizeFormat() const
{
    if (isDir()) {
        return QStringLiteral("-");
    }

    qlonglong fileSize(size());
    bool withUnitVisible = true;
    int forceUnit = -1;

    if (fileSize < 0) {
        qWarning() << "Negative number passed to formatSize():" << fileSize;
        fileSize = 0;
    }

    bool isForceUnit = false;
    QStringList list { " B", " KB", " MB", " GB", " TB" };

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
    QString &&path { filePath() };
    if (SystemPathUtil::instance()->isSystemPath(path)) {
        QString displayName { SystemPathUtil::instance()->systemPathDisplayNameByPath(path) };
        if (!displayName.isEmpty())
            return displayName;
    }
    return fileName();
}

/*!
 * \brief toQFileInfo 获取他的QFileInfo实例对象
 *
 * \return QFileInfo 文件的QFileInfo实例
 */
QFileInfo LocalFileInfo::toQFileInfo() const
{
    QFileInfo info = QFileInfo(d->url.path());
    return info;
}
/*!
 * \brief extraProperties 获取文件的扩展属性
 *
 * \return QVariantHash 文件的扩展属性Hash
 */
QVariantHash LocalFileInfo::extraProperties() const
{
    if (d->extraProperties.isEmpty()) {
    }
    return d->extraProperties;
}

QIcon LocalFileInfo::fileIcon() const
{
    const QUrl &fileUrl = this->url();

    if (FileUtils::containsCopyingFileUrl(fileUrl))
        return QIcon();

    const QString &filePath = this->absoluteFilePath();

    if (!d->fileIcon().isNull() && !d->needThumbnail && (!d->iconFromTheme || !d->fileIcon().name().isEmpty())) {
        return d->fileIcon();
    }

    d->iconFromTheme = false;

#ifdef DFM_MINIMUM
    d->hasThumbnail = 0;
#else
    if (d->hasThumbnail < 0) {
        d->hasThumbnail = FileUtils::isLocalDevice(fileUrl) && DThumbnailProvider::instance()->hasThumbnail(filePath) && !FileUtils::isCdRomDevice(QUrl::fromLocalFile(filePath));
    }
#endif
    if (d->needThumbnail || d->hasThumbnail > 0) {
        d->needThumbnail = true;

        const QIcon icon(DThumbnailProvider::instance()->thumbnailFilePath(filePath, DThumbnailProvider::kLarge));

        if (!icon.isNull()) {
            QPixmap pixmap = icon.pixmap(DThumbnailProvider::kLarge, DThumbnailProvider::kLarge);
            QPainter pa(&pixmap);

            pa.setPen(Qt::gray);
            pa.drawPixmap(0, 0, pixmap);
            QIcon fileIcon = d->fileIcon();
            fileIcon.addPixmap(pixmap);
            d->iconFromTheme = false;
            d->needThumbnail = false;

            d->setIcon(fileIcon);

            return fileIcon;
        }

        if (d->getIconTimer) {
            QMetaObject::invokeMethod(d->getIconTimer, "start", Qt::QueuedConnection);
        } else {
            QTimer *timer = new QTimer();

            QPointer<LocalFileInfo> me = const_cast<LocalFileInfo *>(this);

            d->getIconTimer = timer;
            timer->setSingleShot(true);
            timer->moveToThread(qApp->thread());
            timer->setInterval(REQUEST_THUMBNAIL_DEALY);

            QObject::connect(timer, &QTimer::timeout, [timer, filePath, me] {
                DThumbnailProvider::instance()->appendToProduceQueue(filePath, DThumbnailProvider::kLarge, [me](const QString &path) {
                    if (me) {
                        if (path.isEmpty()) {
                            me->d->iconFromTheme = true;
                        } else {
                            me->d->setIcon(QIcon());
                        }

                        me->d->needThumbnail = false;

                        me->notifyAttributeChanged();
                    } else {
                        qWarning() << "me is nullptr !!!";
                    }
                });
                timer->deleteLater();
            });

            QMetaObject::invokeMethod(timer, "start", Qt::QueuedConnection);
        }

        if (d->fileIcon().isNull())
            d->setIcon(LocalFileIconProvider::globalProvider()->icon(filePath));

        return d->fileIcon();
    } else {
        d->needThumbnail = false;
    }

    if (isSymLink()) {
        const QString &symLinkTarget = this->symLinkTarget();

        if (symLinkTarget != filePath) {
            AbstractFileInfoPointer info = InfoFactory::create<AbstractFileInfo>(QUrl::fromLocalFile(symLinkTarget));
            if (info) {
                d->setIcon(info->fileIcon());
                d->iconFromTheme = false;

                return d->fileIcon();
            }
        }
    }

    d->setIcon(LocalFileIconProvider::globalProvider()->icon(filePath));
    d->iconFromTheme = true;

    return d->fileIcon();
}

QString LocalFileInfo::iconName() const
{
    QReadLocker locker(&d->lock);
    QString iconName = genericIconName();

    if (d->attributes.count(DFileInfo::AttributeID::StandardIcon) == 0) {
        bool success = false;
        QList<QString> data;
        if (d->dfmFileInfo) {
            data = d->dfmFileInfo->attribute(DFileInfo::AttributeID::StandardIcon, &success).value<QList<QString>>();
        }
        if (!data.empty())
            iconName = data.first();
        d->attributes.insert(DFileInfo::AttributeID::StandardIcon, iconName);
    } else {
        iconName = d->attributes.value(DFileInfo::AttributeID::StandardIcon).toString();
    }

    return iconName;
}

QString LocalFileInfo::genericIconName() const
{
    return fileMimeType().genericIconName();
}
/*!
 * \brief inode linux系统下的唯一表示符
 *
 * \return quint64 文件的inode
 */
quint64 LocalFileInfo::inode() const
{
    QReadLocker locker(&d->lock);
    quint64 inNode = d->inode;
    if (d->inode != 0) {
        return inNode;
    }

    struct stat statinfo;
    int filestat = stat(absoluteFilePath().toStdString().c_str(), &statinfo);
    if (filestat != 0) {
        return 0;
    }
    d->inode = statinfo.st_ino;
    return d->inode;
}

QMimeType LocalFileInfo::fileMimeType() const
{
    return MimeDatabase::mimeTypeForUrl(d->url);
}

QString LocalFileInfo::emptyDirectoryTip() const
{
    if (!exists()) {
        return QObject::tr("File has been moved or deleted");
    } else if (!isReadable()) {
        return QObject::tr("You do not have permission to access this folder");
    } else if (isDir()) {
        if (!isExecutable())
            return QObject::tr("You do not have permission to traverse files in it");
    }

    return AbstractFileInfo::emptyDirectoryTip();
}

bool LocalFileInfo::canDragCompress() const
{
    return isDragCompressFileFormat()
            && isWritable()
            && isReadable();
}

bool LocalFileInfo::isDragCompressFileFormat() const
{
    const QString &&name = fileName();
    return name.endsWith(".zip")
            || (name.endsWith(".7z")
                && !name.endsWith(".tar.7z"));
}

void LocalFileInfo::mediaInfoAttributes(DFileInfo::MediaType type, QList<DFileInfo::AttributeExtendID> ids, DFileInfo::AttributeExtendFuncCallback callback) const
{
    if (d->dfmFileInfo) {
        d->extendIDs = ids;
        d->attributesExtendCallbackFunc = callback;

        auto it = ids.begin();
        while (it != ids.end()) {
            if (d->attributesExtend.count(*it))
                it = ids.erase(it);
            else
                ++it;
        }

        if (!ids.isEmpty()) {
            d->dfmFileInfo->attributeExtend(type, ids, std::bind(&LocalFileInfoPrivate::attributesExtendCallback, d, std::placeholders::_1, std::placeholders::_2));
        } else {
            d->attributesExtendCallback(true, {});
        }
    }
}

bool LocalFileInfo::notifyAttributeChanged()
{
    if (d->dfmFileInfo)
        return d->dfmFileInfo->setCustomAttribute("xattr::update", DFMIO::DFileInfo::DFileAttributeType::TypeString, "");

    return false;
}

QString LocalFileInfo::mimeTypeName() const
{
    QReadLocker locker(&d->lock);
    QString type;

    if (d->attributes.count(DFileInfo::AttributeID::StandardContentType) == 0) {
        bool success = false;
        if (d->dfmFileInfo) {
            type = d->dfmFileInfo->attribute(DFileInfo::AttributeID::StandardContentType, &success).toString();
        }
        if (!success)
            type = fileMimeType().name();
        d->attributes.insert(DFileInfo::AttributeID::StandardContentType, type);
    } else {
        type = d->attributes.value(DFileInfo::AttributeID::StandardContentType).toString();
    }
    return type;
}

void LocalFileInfo::init(const QUrl &url)
{
    if (url.isEmpty()) {
        qWarning("Failed, can't use empty url init fileinfo");
        abort();
    }

    if (UrlRoute::isVirtual(url)) {
        qWarning("Failed, can't use virtual scheme init local fileinfo");
        abort();
    }

    QUrl cvtResultUrl = QUrl::fromLocalFile(UrlRoute::urlToPath(url));

    if (!url.isValid()) {
        qWarning("Failed, can't use valid url init fileinfo");
        abort();
    }

    QSharedPointer<DIOFactory> factory = produceQSharedIOFactory(cvtResultUrl.scheme(), static_cast<QUrl>(cvtResultUrl));
    if (!factory) {
        qWarning("Failed, dfm-io create factory");
        abort();
    }

    d->dfmFileInfo = factory->createFileInfo();
    if (!d->dfmFileInfo) {
        qWarning("Failed, dfm-io use factory create fileinfo");
        abort();
    }
}

DFMBASE_END_NAMESPACE
