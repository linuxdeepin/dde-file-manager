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
#include "dfileiconprovider.h"

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

/*!
 * \class LocalFileInfo 本地文件信息类
 * \brief 内部实现本地文件的fileinfo，对应url的scheme是file://
 */
DWIDGET_USE_NAMESPACE
DFMBASE_BEGIN_NAMESPACE

LocalFileInfo::LocalFileInfo(const QUrl &url)
    : AbstractFileInfo(url), d(new LocalFileInfoPrivate(this))
{
    d->url = url;
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

LocalFileInfo::~LocalFileInfo()
{
}

LocalFileInfo &LocalFileInfo::operator=(const LocalFileInfo &info)
{
    AbstractFileInfo::operator=(info);
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
void LocalFileInfo::setFile(const DFileInfo &file)
{
    d->url = file.uri();
    d->dfmFileInfo.reset(new DFileInfo(file));
    refresh();
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
    if (!d->caches.contains(kTypeExists)) {
        if (d->dfmFileInfo) {
            d->caches.insert(kTypeExists, QVariant(d->dfmFileInfo->exists()));
        } else {
            return false;
        }
    }
    return d->caches.value(kTypeExists).toBool();
}
/*!
 * \brief refresh 跟新文件信息，清理掉缓存的所有的文件信息
 *
 * \param
 *
 * \return
 */
void LocalFileInfo::refresh()
{
    d->caches.clear();
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
    //文件的路径（当前文件的父目录）
    if (!d->caches.contains(kTypeFilePath)) {
        if (d->dfmFileInfo) {
            bool success = false;
            d->caches.insert(kTypeFilePath, d->dfmFileInfo->attribute(DFileInfo::AttributeID::StandardFilePath, &success));
            if (!success)
                qWarning() << "get dfm-io DFileInfo StandardFilePath failed!";
        } else {
            return QString();
        }
    }
    return d->caches.value(kTypeFilePath).toString();
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
    if (!d->caches.contains(kTypeFileName)
        || !d->caches.value(kTypeFileName).isValid()) {
        if (d->dfmFileInfo.isNull()) {
            return QString();
        }

        bool success = false;
        d->caches.insert(kTypeFileName, d->dfmFileInfo->attribute(DFileInfo::AttributeID::StandardName, &success));
        if (!success)
            qWarning() << "get dfm-io DFileInfo StandardName failed!";
    }

    QString string = d->caches.value(kTypeFileName).toString();
    return string;
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
    if (!d->caches.contains(kTypeBaseName)) {
        if (d->dfmFileInfo) {
            bool success = false;
            d->caches.insert(kTypeBaseName, d->dfmFileInfo->attribute(DFileInfo::AttributeID::StandardBaseName, &success));
            if (!success)
                qWarning() << "get dfm-io DFileInfo StandardBaseName failed!";
        } else {
            return QString();
        }
    }
    return d->caches.value(kTypeBaseName).toString();
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
    if (!d->caches.contains(kTypeCompleteBaseName)) {
        if (d->dfmFileInfo) {
            bool success = false;
            d->caches.insert(kTypeCompleteBaseName, d->dfmFileInfo->attribute(DFileInfo::AttributeID::StandardBaseName, &success));
            if (!success)
                qWarning() << "get dfm-io DFileInfo StandardBaseName failed!";
        } else {
            return QString();
        }
    }
    return d->caches.value(kTypeCompleteBaseName).toString();
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
    if (!d->caches.contains(kTypeSuffix)) {
        if (d->dfmFileInfo) {
            bool success = false;
            d->caches.insert(kTypeSuffix, d->dfmFileInfo->attribute(DFileInfo::AttributeID::StandardSuffix, &success));
            if (!success)
                qWarning() << "get dfm-io DFileInfo StandardSuffix failed!";
        } else {
            return QString();
        }
    }
    return d->caches.value(kTypeSuffix).toString();
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
    if (!d->caches.contains(kTypeCompleteSuffix)) {
        if (d->dfmFileInfo) {
            bool success = false;
            d->caches.insert(kTypeCompleteSuffix, d->dfmFileInfo->attribute(DFileInfo::AttributeID::StandardCompleteSuffix, &success));
            if (!success)
                qWarning() << "get dfm-io DFileInfo StandardCompleteSuffix failed!";
        } else {
            return QString();
        }
    }
    return d->caches.value(kTypeCompleteSuffix).toString();
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
    if (!d->caches.contains(kTypePath)) {
        if (d->dfmFileInfo) {
            bool success = false;
            d->caches.insert(kTypePath, d->dfmFileInfo->attribute(DFileInfo::AttributeID::StandardFilePath, &success));
            if (!success)
                qWarning() << "get dfm-io DFileInfo StandardFilePath failed!";
        } else {
            return QString();
        }
    }
    return d->caches.value(kTypePath).toString();
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
    return path();
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
    return UrlRoute::pathToReal(UrlRoute::urlToPath(d->dfmFileInfo->uri()));
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
    if (!d->caches.contains(kTypeIsReadable)) {
        if (d->dfmFileInfo) {
            bool success = false;
            d->caches.insert(kTypeIsReadable, d->dfmFileInfo->attribute(DFileInfo::AttributeID::AccessCanRead, &success));
            if (!success)
                qWarning() << "get dfm-io DFileInfo AccessCanRead failed!";
        } else {
            return false;
        }
    }
    return d->caches.value(kTypeIsReadable).toBool();
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
    if (!d->caches.contains(kTypeIsWritable)) {
        if (d->dfmFileInfo) {
            bool success = false;
            d->caches.insert(kTypeIsWritable, d->dfmFileInfo->attribute(DFileInfo::AttributeID::AccessCanWrite, &success));
            if (!success)
                qWarning() << "get dfm-io DFileInfo AccessCanWrite failed!";
        } else {
            return false;
        }
    }
    return d->caches.value(kTypeIsWritable).toBool();
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
    if (!d->caches.contains(kTypeIsExecutable)) {
        if (d->dfmFileInfo) {
            bool success = false;
            d->caches.insert(kTypeIsExecutable, d->dfmFileInfo->attribute(DFileInfo::AttributeID::AccessCanExecute, &success));
            if (!success)
                qWarning() << "get dfm-io DFileInfo AccessCanExecute failed!";
        } else {
            return false;
        }
    }
    return d->caches.value(kTypeIsExecutable).toBool();
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
    if (!d->caches.contains(kTypeIsHidden)
        || !d->caches.value(kTypeIsHidden).isValid()) {
        if (d->dfmFileInfo) {
            bool success = false;
            d->caches.insert(kTypeIsHidden, d->dfmFileInfo->attribute(DFileInfo::AttributeID::StandardIsHidden, &success));
            if (!success)
                qWarning() << "get dfm-io DFileInfo StandardIsHidden failed!";
        } else {
            return false;
        }
    }
    return d->caches.value(kTypeIsHidden).toBool();
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
    if (!d->caches.contains(kTypeIsFile)) {
        if (d->dfmFileInfo) {
            bool success = false;
            d->caches.insert(kTypeIsFile, d->dfmFileInfo->attribute(DFileInfo::AttributeID::StandardIsFile, &success));
            if (!success)
                qWarning() << "get dfm-io DFileInfo StandardIsFile failed!";
        } else {
            return false;
        }
    }
    return d->caches.value(kTypeIsFile).toBool();
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
    if (!d->caches.contains(kTypeIsDir)) {
        if (d->dfmFileInfo) {
            bool success = true;
            d->caches.insert(kTypeIsDir, d->dfmFileInfo->attribute(DFileInfo::AttributeID::StandardIsDir, &success));
            if (!success)
                qWarning() << "get dfm-io DFileInfo StandardIsDir failed!";
        } else {
            return false;
        }
    }
    return d->caches.value(kTypeIsDir).toBool();
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
    if (!d->caches.contains(kTypeIsSymLink)) {
        if (d->dfmFileInfo) {
            bool success = true;
            d->caches.insert(kTypeIsDir, d->dfmFileInfo->attribute(DFileInfo::AttributeID::StandardIsSymlink, &success));
            if (!success)
                qWarning() << "get dfm-io DFileInfo StandardIsSymlink failed!";
        } else {
            return false;
        }
    }
    return d->caches.value(kTypeIsSymLink).toBool();
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
    if (!d->caches.contains(kTypeSymLinkTarget)) {
        if (d->dfmFileInfo) {
            bool success(false);
            d->caches.insert(kTypeSymLinkTarget, d->dfmFileInfo->attribute(DFileInfo::AttributeID::StandardSymlinkTarget, &success));
            if (!success)
                qWarning() << "get dfm-io DFileInfo StandardSymlinkTarget failed!";
        } else {
            return QString();
        }
    }
    return d->caches.value(kTypeSymLinkTarget).toString();
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
    if (!d->caches.contains(kTypeOwner)) {
        if (d->dfmFileInfo) {
            bool success(false);
            d->caches.insert(kTypeOwner, d->dfmFileInfo->attribute(DFileInfo::AttributeID::OwnerUser, &success));
            if (!success)
                qWarning() << "get dfm-io DFileInfo OwnerUser failed!";
        } else {
            return QString();
        }
    }
    return d->caches.value(kTypeOwner).toString();
}
/*!
 * \brief ownerId 获取文件的拥有者ID
 *
 * Returns the id of the owner of the file.
 *
 * \param
 *
 * \return uint 文件的拥有者ID
 */
uint LocalFileInfo::ownerId() const
{
    if (!d->caches.contains(kTypeOwnerID)) {
        if (d->dfmFileInfo) {
            bool success(false);
            d->caches.insert(kTypeOwnerID, d->dfmFileInfo->attribute(DFileInfo::AttributeID::OwnerUser, &success));
            if (!success)
                qWarning() << "get dfm-io DFileInfo OwnerUser failed!";
        } else {
            return static_cast<uint>(-1);
        }
    }
    return d->caches.value(kTypeOwnerID).toUInt();
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
    if (!d->caches.contains(kTypeGroup)) {
        if (d->dfmFileInfo) {
            bool success(false);
            d->caches.insert(kTypeGroup, d->dfmFileInfo->attribute(DFileInfo::AttributeID::OwnerGroup, &success));
            if (!success)
                qWarning() << "get dfm-io DFileInfo OwnerGroup failed!";
        } else {
            return QString();
        }
    }
    return d->caches.value(kTypeGroup).toString();
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
    if (!d->caches.contains(kTypeGroupID)) {
        if (d->dfmFileInfo) {
            bool success(false);
            d->caches.insert(kTypeGroupID, d->dfmFileInfo->attribute(DFileInfo::AttributeID::OwnerGroup, &success));
            if (!success)
                qWarning() << "get dfm-io DFileInfo OwnerGroup failed!";
        } else {
            return static_cast<uint>(-1);
        }
    }
    return d->caches.value(kTypeGroupID).toUInt();
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
    if (!d->caches.contains(kTypePermissions)) {
        if (d->dfmFileInfo) {
            // @todo 目前dfm-io还没实现，等待实现
            //            bool success(false);
            //            d->caches.insert(TypePermissions,d->dfmFileInfo->
            //                                           attribute(DFileInfo::AttributeID::OwnerGroup, &success));
        } else {
            QFileDevice::Permissions ps;
            return ps;
        }
    }
    return static_cast<QFileDevice::Permissions>(d->caches.value(kTypePermissions).toInt());
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
    if (!d->caches.contains(kTypeSize)) {
        if (d->dfmFileInfo) {
            bool success(false);
            d->caches.insert(kTypeSize, d->dfmFileInfo->attribute(DFileInfo::AttributeID::StandardSize, &success));
            if (!success)
                qWarning() << "get dfm-io DFileInfo StandardSize failed!";
        } else {
            return 0;
        }
    }
    return static_cast<QFileDevice::Permissions>(d->caches.value(kTypeSize).toInt());
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
    if (!d->caches.contains(kTypeCreateTime)) {
        if (d->dfmFileInfo) {
            bool success(false);
            d->caches.insert(kTypeCreateTime, d->dfmFileInfo->attribute(DFileInfo::AttributeID::TimeCreated, &success));
            if (!success)
                qWarning() << "get dfm-io DFileInfo TimeCreated failed!";
        } else {
            return QDateTime();
        }
    }
    uint64_t data = d->caches.value(kTypeCreateTime).value<uint64_t>();
    return QDateTime::fromTime_t(static_cast<uint>(data));
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
    if (!d->caches.contains(kTypeChangeTime)) {
        if (d->dfmFileInfo) {
            bool success(false);
            d->caches.insert(kTypeChangeTime, d->dfmFileInfo->attribute(DFileInfo::AttributeID::TimeChanged, &success));
            if (!success)
                qWarning() << "get dfm-io DFileInfo TimeChanged failed!";
        } else {
            return QDateTime();
        }
    }
    uint64_t data = d->caches.value(kTypeChangeTime).value<uint64_t>();
    return QDateTime::fromTime_t(static_cast<uint>(data));
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
    if (!d->caches.contains(kTypeLastModifyTime)) {
        if (d->dfmFileInfo) {
            bool success(false);
            d->caches.insert(kTypeLastModifyTime, d->dfmFileInfo->attribute(DFileInfo::AttributeID::TimeModified, &success));
            if (!success)
                qWarning() << "get dfm-io DFileInfo TimeModified failed!";
        } else {
            return QDateTime();
        }
    }
    uint64_t data = d->caches.value(kTypeLastModifyTime).value<uint64_t>();
    return QDateTime::fromTime_t(static_cast<uint>(data));
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
    if (!d->caches.contains(kTypeLastReadTime)) {
        if (d->dfmFileInfo) {
            bool success(false);
            d->caches.insert(kTypeLastReadTime, d->dfmFileInfo->attribute(DFileInfo::AttributeID::TimeAccess, &success));
            if (!success)
                qWarning() << "get dfm-io DFileInfo TimeAccess failed!";
        } else {
            return QDateTime();
        }
    }
    uint64_t data = d->caches.value(kTypeLastReadTime).value<uint64_t>();
    return QDateTime::fromTime_t(static_cast<uint>(data));
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
    // TODO::
    if (!isBlockDev())
        return "";
    else
        return "";
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
LocalFileInfo::Type LocalFileInfo::fileType() const
{
    if (d->fileType != MimeDatabase::FileType::kUnknown)
        return Type(d->fileType);

    QString absoluteFilePath = filePath();
    if (absoluteFilePath.startsWith(StandardPaths::location(StandardPaths::kTrashFilesPath))
        && isSymLink()) {
        d->fileType = MimeDatabase::FileType::kRegularFile;
        return Type(d->fileType);
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

    return Type(d->fileType);
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
 * \return QString 大小格式化后的大小
 */
QString LocalFileInfo::sizeFormat() const
{
    if (isDir()) {
        return QStringLiteral("-");
    }

    qreal fileSize(size());
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
    return fileName();
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
    if (d->extraProperties.isEmpty()) {
    }
    return d->extraProperties;
}

QIcon LocalFileInfo::fileIcon() const
{
    return DFileIconProvider::globalProvider()->icon(this->path());
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
