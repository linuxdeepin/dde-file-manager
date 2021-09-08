/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
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
#include "private/abstractfileinfo_p.h"
#include "abstractfileinfo.h"

#include <QMetaType>
#include <QDateTime>
#include <QVariant>
#include <QDir>

USING_IO_NAMESPACE

DFMBASE_BEGIN_NAMESPACE
Q_GLOBAL_STATIC_WITH_ARGS(int,type_id,{qRegisterMetaType<AbstractFileInfoPointer>("AbstractFileInfo")});

/*!
 * \class DAbstractFileInfo 抽象文件信息类
 *
 * \brief 内部实现Url到真实路径的信息关联，设置的真实本地路径总是指向虚拟路径Url
 *
 * 在这之前你应该明确你的路径是否被DFMUrlRoute注册，如果没有注册那么Url会指向QUrl标准定义：
 *
 * QUrl(file:///root) 标识/root路径
 */

/*!
 * \brief DAbstractFileInfo 构造函数
 *
 * \param QUrl & 文件的URL
 */
AbstractFileInfo::AbstractFileInfo(const QUrl &url)
    :d_ptr(new AbstractFileInfoPrivate(this))
{
    Q_D(AbstractFileInfo);
    d->url = QUrl::fromLocalFile(UrlRoute::urlToPath(url));
    d->initFileInfo();
}

AbstractFileInfo::~AbstractFileInfo()
{

}
/*!
 * \brief = 重载操作符=
 *
 * \param const DAbstractFileInfo & DAbstractFileInfo实例对象的引用
 *
 * \return DAbstractFileInfo & 新DAbstractFileInfo实例对象的引用
 */
AbstractFileInfo &AbstractFileInfo::operator=(const AbstractFileInfo &fileinfo)
{
    d_ptr->dfmFileInfo = fileinfo.d_ptr->dfmFileInfo;
    return *this;
}
/*!
 * \brief == 重载操作符==
 *
 * \param const DAbstractFileInfo & DAbstractFileInfo实例对象的引用
 *
 * \return bool 传入的DAbstractFileInfo实例对象和自己是否相等
 */
bool AbstractFileInfo::operator==(const AbstractFileInfo &fileinfo) const
{

    return (d_ptr->dfmFileInfo == fileinfo.d_ptr->dfmFileInfo);
}
/*!
 * \brief != 重载操作符!=
 *
 * \param const DAbstractFileInfo & DAbstractFileInfo实例对象的引用
 *
 * \return bool 传入的DAbstractFileInfo实例对象和自己是否不相等
 */
bool AbstractFileInfo::operator!=(const AbstractFileInfo &fileinfo) const
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
void AbstractFileInfo::setFile(const DFileInfo &file)
{
    Q_D(AbstractFileInfo);
    d->url = file.uri();
    d_ptr->dfmFileInfo.reset(new DFileInfo(file));
    refresh();
}

/*!
 * \brief setFile 设置文件的File，跟新当前的fileinfo
 *
 * \param const QUrl &url 新文件的URL
 *
 * \return
 */
void AbstractFileInfo::setFile(const QUrl &url)
{
    Q_D(AbstractFileInfo);
    d->url = QUrl::fromLocalFile(UrlRoute::urlToPath(url));
    d->initFileInfo();
    this->refresh();
}
/*!
 * \brief exists 文件是否存在
 *
 * \param
 *
 * \return 返回文件是否存在
 */
bool AbstractFileInfo::exists() const
{


    if (!d_ptr->caches.contains(TypeExists)) {
        if (d_ptr->dfmFileInfo) {
            // 目前dfmio这一层还没实现，等待实现了在加入
//            d_ptr->m_caches.insert(TypeExists,QVariant(d_ptr->m_dfmFileInfo->attribute()));
        } else {
            return false;
        }
    }
    return d_ptr->caches.value(TypeExists).toBool();
}
/*!
 * \brief refresh 跟新文件信息，清理掉缓存的所有的文件信息
 *
 * \param
 *
 * \return
 */
void AbstractFileInfo::refresh()
{
    Q_D(AbstractFileInfo);
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
QString AbstractFileInfo::filePath() const
{

    //文件的路径（当前文件的父目录）

    if (!d_ptr->caches.contains(TypeFilePath)) {
        if (d_ptr->dfmFileInfo) {
//            d_ptr->m_caches.insert(TypeFilePath, QVariant(DFMUrlRoute::urlToPath(d->m_url)));
        } else {
            return QString();
        }
    }
    return d_ptr->caches.value(TypeFilePath).toString();
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
QString AbstractFileInfo::absoluteFilePath() const
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
QString AbstractFileInfo::fileName() const
{



    if (!d_ptr->caches.contains(TypeFileName)) {
        if (d_ptr->dfmFileInfo) {
            bool success = false;
            d_ptr->caches.insert(TypeFileName, d_ptr->dfmFileInfo->
                                  attribute(DFileInfo::AttributeID::StandardName, success));
        } else {
            return QString();
        }
    }
    return d_ptr->caches.value(TypeFileName).toString();
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
QString AbstractFileInfo::baseName() const
{


    if (!d_ptr->caches.contains(TypeBaseName)) {
        if (d_ptr->dfmFileInfo) {
//            bool success = false;
//            d_ptr->m_caches.insert(TypeBaseName, QVariant(d_ptr->m_dfmFileInfo->attribute(DFileInfo::AttributeID::StandardName, success)));
        } else {
            return QString();
        }
    }
    return d_ptr->caches.value(TypeBaseName).toString();
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
QString AbstractFileInfo::completeBaseName() const
{


    if (!d_ptr->caches.contains(TypeCompleteBaseName)) {
        if (d_ptr->dfmFileInfo) {
//            bool success = false;
//            d_ptr->m_caches.insert(TypeCompleteBaseName, QVariant(d_ptr->m_dfmFileInfo->attribute(DFileInfo::AttributeID::StandardName, success)));
        } else {
            return QString();
        }
    }
    return d_ptr->caches.value(TypeCompleteBaseName).toString();
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
QString AbstractFileInfo::suffix() const
{


    if (!d_ptr->caches.contains(TypeSuffix)) {
        if (d_ptr->dfmFileInfo) {
//            bool success = false;
//            d_ptr->m_caches.insert(TypeSuffix, QVariant(d_ptr->m_dfmFileInfo->attribute(DFileInfo::AttributeID::StandardName, success)));
        } else {
            return QString();
        }
    }
    return d_ptr->caches.value(TypeSuffix).toString();
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
QString AbstractFileInfo::completeSuffix()
{


    if (!d_ptr->caches.contains(TypeCompleteSuffix)) {
        if (d_ptr->dfmFileInfo) {
//            bool success = false;
//            d_ptr->m_caches.insert(TypeCompleteSuffix, QVariant(d_ptr->m_dfmFileInfo->attribute(DFileInfo::AttributeID::StandardName, success)));
        } else {
            return QString();
        }
    }
    return d_ptr->caches.value(TypeCompleteSuffix).toString();
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
QString AbstractFileInfo::path() const
{


    if (!d_ptr->caches.contains(TypePath)) {
        if (d_ptr->dfmFileInfo) {
//            bool success = false;
//            d_ptr->m_caches.insert(TypePath, QVariant(d_ptr->m_dfmFileInfo->attribute(DFileInfo::AttributeID::StandardName, success)));
        } else {
            return QString();
        }
    }
    return d_ptr->caches.value(TypePath).toString();
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
QString AbstractFileInfo::absolutePath() const
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
QString AbstractFileInfo::canonicalPath() const
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
QDir AbstractFileInfo::dir() const
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
QDir AbstractFileInfo::absoluteDir() const
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
QUrl AbstractFileInfo::url() const
{
    return UrlRoute::pathToUrl(filePath());
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
bool AbstractFileInfo::isReadable() const
{


    if (!d_ptr->caches.contains(TypeIsReadable)) {
        if (d_ptr->dfmFileInfo) {
            bool success = false;
            d_ptr->caches.insert(TypeIsReadable,d_ptr->dfmFileInfo->
                                  attribute(DFileInfo::AttributeID::AccessCanRead, success));
        } else {
            return false;
        }
    }
    return d_ptr->caches.value(TypeIsReadable).toBool();
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
bool AbstractFileInfo::isWritable() const
{
    if (!d_ptr->caches.contains(TypeIsWritable)) {
        if (d_ptr->dfmFileInfo) {
            bool success = false;
            d_ptr->caches.insert(TypeIsWritable,d_ptr->dfmFileInfo->
                                  attribute(DFileInfo::AttributeID::AccessCanWrite, success));
        } else {
            return false;
        }
    }
    return d_ptr->caches.value(TypeIsWritable).toBool();
}
/*!
 * \brief isExecutable 获取文件是否可执行
 *
 * \param
 *
 * \return bool 返回文件是否可执行
 */
bool AbstractFileInfo::isExecutable() const
{
    if (!d_ptr->caches.contains(TypeIsExecutable)) {
        if (d_ptr->dfmFileInfo) {
            bool success = false;
            d_ptr->caches.insert(TypeIsExecutable, d_ptr->dfmFileInfo->
                                  attribute(DFileInfo::AttributeID::AccessCanExecute, success));
        } else {
            return false;
        }
    }
    return d_ptr->caches.value(TypeIsExecutable).toBool();
}
/*!
 * \brief isHidden 获取文件是否是隐藏
 *
 * \param
 *
 * \return bool 返回文件是否隐藏
 */
bool AbstractFileInfo::isHidden() const
{
    if (!d_ptr->caches.contains(TypeIsHidden)) {
        if (d_ptr->dfmFileInfo) {
            bool success = false;
            d_ptr->caches.insert(TypeIsHidden,
                                  d_ptr->dfmFileInfo->
                                  attribute(DFileInfo::AttributeID::StandardIsHiden, success));
        } else {
            return false;
        }
    }
    return d_ptr->caches.value(TypeIsHidden).toBool();
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
bool AbstractFileInfo::isFile() const
{
    if (!d_ptr->caches.contains(TypeIsFile)) {
        if (d_ptr->dfmFileInfo) {
//            d_ptr->m_caches.insert(TypeIsFile, QVariant(DFMUrlRoute::urlToPath(d->m_url)));
        } else {
            return false;
        }
    }
    return d_ptr->caches.value(TypeIsFile).toBool();
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
bool AbstractFileInfo::isDir() const
{
    if (!d_ptr->caches.contains(TypeIsDir)) {
        if (d_ptr->dfmFileInfo) {
//            d_ptr->m_caches.insert(TypeIsDir, QVariant(DFMUrlRoute::urlToPath(d->m_url)));
        } else {
            return false;
        }
    }
    return d_ptr->caches.value(TypeIsDir).toBool();
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
bool AbstractFileInfo::isSymLink() const
{
    if (!d_ptr->caches.contains(TypeIsSymLink)) {
        if (d_ptr->dfmFileInfo) {
//            d_ptr->m_caches.insert(TypeisSymLink, QVariant(DFMUrlRoute::urlToPath(d->m_url)));
        } else {
            return false;
        }
    }
    return d_ptr->caches.value(TypeIsSymLink).toBool();
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
bool AbstractFileInfo::isRoot() const
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
bool AbstractFileInfo::isBundle() const
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
QString AbstractFileInfo::symLinkTarget() const
{
    if (!d_ptr->caches.contains(TypeSymLinkTarget)) {
        if (d_ptr->dfmFileInfo) {
            bool success(false);
            d_ptr->caches.insert(TypeSymLinkTarget,d_ptr->dfmFileInfo->
                                  attribute(DFileInfo::AttributeID::StandardSymlinkTarget, success));
        } else {
            return QString();
        }
    }
    return d_ptr->caches.value(TypeSymLinkTarget).toString();
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
QString AbstractFileInfo::owner() const
{
    if (!d_ptr->caches.contains(TypeOwner)) {
        if (d_ptr->dfmFileInfo) {
            bool success(false);
            d_ptr->caches.insert(TypeOwner, d_ptr->dfmFileInfo->
                                  attribute(DFileInfo::AttributeID::OwnerUser, success));
        } else {
            return QString();
        }
    }
    return d_ptr->caches.value(TypeOwner).toString();
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
uint AbstractFileInfo::ownerId() const
{
    if (!d_ptr->caches.contains(TypeOwnerID)) {
        if (d_ptr->dfmFileInfo) {
            bool success(false);
            d_ptr->caches.insert(TypeOwnerID, d_ptr->dfmFileInfo->
                                  attribute(DFileInfo::AttributeID::OwnerUser, success));
        } else {
            return static_cast<uint>(-1);
        }
    }
    return d_ptr->caches.value(TypeOwnerID).toUInt();
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
QString AbstractFileInfo::group() const
{
    if (!d_ptr->caches.contains(TypeGroup)) {
        if (d_ptr->dfmFileInfo) {
            bool success(false);
            d_ptr->caches.insert(TypeGroup, d_ptr->dfmFileInfo->
                                  attribute(DFileInfo::AttributeID::OwnerGroup, success));
        } else {
            return QString();
        }
    }
    return d_ptr->caches.value(TypeGroup).toString();
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
uint AbstractFileInfo::groupId() const
{
    if (!d_ptr->caches.contains(TypeGroupID)) {
        if (d_ptr->dfmFileInfo) {
            bool success(false);
            d_ptr->caches.insert(TypeGroupID, d_ptr->dfmFileInfo->
                                  attribute(DFileInfo::AttributeID::OwnerGroup, success));
        } else {
            return static_cast<uint>(-1);
        }
    }
    return d_ptr->caches.value(TypeGroupID).toUInt();
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
bool AbstractFileInfo::permission(QFileDevice::Permissions permissions) const
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
QFileDevice::Permissions AbstractFileInfo::permissions() const
{
    if (!d_ptr->caches.contains(TypePermissions)) {
        if (d_ptr->dfmFileInfo) {
//            bool success(false);
//            d_ptr->m_caches.insert(TypePermissions,
//                                  QVariant(d_ptr->m_dfmFileInfo->
//                                           attribute(DFileInfo::AttributeID::OwnerGroup, success)));
        } else {
            QFileDevice::Permissions ps;
            return ps;
        }
    }
    return static_cast<QFileDevice::Permissions>(d_ptr->caches.value(TypePermissions).toInt());
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
qint64 AbstractFileInfo::size() const
{
    if (!d_ptr->caches.contains(TypeSize)) {
        if (d_ptr->dfmFileInfo) {
            bool success(false);
            d_ptr->caches.insert(TypeSize,d_ptr->dfmFileInfo->
                                  attribute(DFileInfo::AttributeID::StandardSize, success));
        } else {
            return 0;
        }
    }
    return static_cast<QFileDevice::Permissions>(d_ptr->caches.value(TypeSize).toInt());
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
QDateTime AbstractFileInfo::created() const
{
    if (!d_ptr->caches.contains(TypeCreateTime)) {
        if (d_ptr->dfmFileInfo) {
            bool success(false);
            d_ptr->caches.insert(TypeCreateTime, d_ptr->dfmFileInfo->
                                  attribute(DFileInfo::AttributeID::TimeCreated, success));
        } else {
            return QDateTime();
        }
    }
    QString data = d_ptr->caches.value(TypeCreateTime).toString();
    QStringList dataList;
    if (data.isEmpty()) {
        dataList << "00" << "00" << "00";
    } else {
        dataList = data.split(":");
        while (dataList.count() < 3) {
            dataList << "0";
        }
    }
    return QDateTime(QDate(dataList.at(0).toInt(), dataList.at(1).toInt(),dataList.at(2).toInt()));
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
QDateTime AbstractFileInfo::birthTime() const
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
QDateTime AbstractFileInfo::metadataChangeTime() const
{
    if (!d_ptr->caches.contains(TypeChangeTime)) {
        if (d_ptr->dfmFileInfo) {
            bool success(false);
            d_ptr->caches.insert(TypeChangeTime,d_ptr->dfmFileInfo->
                                  attribute(DFileInfo::AttributeID::TimeChanged, success));
        } else {
            return QDateTime();
        }
    }
    QString data = d_ptr->caches.value(TypeChangeTime).toString();
    QStringList dataList;
    if (data.isEmpty()) {
        dataList << "00" << "00" << "00";
    } else {
        dataList = data.split(":");
        while (dataList.count() < 3) {
            dataList << "0";
        }
    }
    return QDateTime(QDate(dataList.at(0).toInt(), dataList.at(1).toInt(),dataList.at(2).toInt()));
}
/*!
 * \brief lastModified 获取文件的最后修改时间
 *
 * \param
 *
 * \return QDateTime 文件的最后修改时间的QDateTime实例
 */
QDateTime AbstractFileInfo::lastModified() const
{
    if (!d_ptr->caches.contains(TypeLastModifyTime)) {
        if (d_ptr->dfmFileInfo) {
            bool success(false);
            d_ptr->caches.insert(TypeLastModifyTime, d_ptr->dfmFileInfo->
                                  attribute(DFileInfo::AttributeID::TimeModified, success));
        } else {
            return QDateTime();
        }
    }
    QString data = d_ptr->caches.value(TypeLastModifyTime).toString();
    QStringList dataList;
    if (data.isEmpty()) {
        dataList << "00" << "00" << "00";
    } else {
        dataList = data.split(":");
        while (dataList.count() < 3) {
            dataList << "0";
        }
    }
    return QDateTime(QDate(dataList.at(0).toInt(), dataList.at(1).toInt(),dataList.at(2).toInt()));
}
/*!
 * \brief lastRead 获取文件的最后读取时间
 *
 * \param
 *
 * \return QDateTime 文件的最后读取时间的QDateTime实例
 */
QDateTime AbstractFileInfo::lastRead() const
{
    if (!d_ptr->caches.contains(TypeLastReadTime)) {
        if (d_ptr->dfmFileInfo) {
            bool success(false);
            d_ptr->caches.insert(TypeLastReadTime, d_ptr->dfmFileInfo->
                                  attribute(DFileInfo::AttributeID::TimeAccess, success));
        } else {
            return QDateTime();
        }
    }
    QString data = d_ptr->caches.value(TypeLastReadTime).toString();
    QStringList dataList;
    if (data.isEmpty()) {
        dataList << "00" << "00" << "00";
    } else {
        dataList = data.split(":");
        while (dataList.count() < 3) {
            dataList << "0";
        }
    }
    return QDateTime(QDate(dataList.at(0).toInt(), dataList.at(1).toInt(),dataList.at(2).toInt()));
}
/*!
 * \brief fileTime 获取文件的事件通过传入的参数
 *
 * \param QFile::FileTime time 时间类型
 *
 * \return QDateTime 文件的不同时间类型的时间的QDateTime实例
 */
QDateTime AbstractFileInfo::fileTime(QFileDevice::FileTime time) const
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
 * \class DAbstractFileInfoPrivate 抽象文件信息私有类
 *
 * \brief 主要存储文件信息的成员变量和数据
 */
AbstractFileInfoPrivate::AbstractFileInfoPrivate(AbstractFileInfo *qq)
    : q_ptr(qq)
{

}

AbstractFileInfoPrivate::~AbstractFileInfoPrivate() {

}

/*!
 * \brief AbstractFileInfo::initFileInfo 创建dfmio中dfileinfo
 */
void AbstractFileInfoPrivate::initFileInfo()
{
    QSharedPointer<DIOFactory> factory = produceQSharedIOFactory(url.scheme(), QUrl(url));
    if (factory.isNull()) {
        qWarning("create factory failed.");
        abort();
    }
    dfmFileInfo = factory->createFileInfo();
    if (!dfmFileInfo) {
        qWarning("create fileinfo failed.");
        return;
    }
}
DFMBASE_END_NAMESPACE
