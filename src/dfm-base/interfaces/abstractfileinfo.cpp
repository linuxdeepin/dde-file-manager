/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
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
#include "private/abstractfileinfo_p.h"
#include "abstractfileinfo.h"
#include "utils/chinese2pinyin.h"
#include "dfm-base/mimetype/mimetypedisplaymanager.h"
#include "dfm-base/utils/fileutils.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/utils/universalutils.h"

#include <dfm-io/dfmio_utils.h>

#include <QMetaType>
#include <QDateTime>
#include <QVariant>
#include <QDir>

USING_IO_NAMESPACE

#define CALL_PROXY(Fun) \
    if (dptr->proxy) return dptr->proxy->Fun;
namespace dfmbase {
Q_GLOBAL_STATIC_WITH_ARGS(int, type_id, { qRegisterMetaType<AbstractFileInfoPointer>("AbstractFileInfo") })

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
    : dptr(new AbstractFileInfoPrivate(url, this))
{
    Q_UNUSED(type_id)
    qRegisterMetaType<QMap<dfmio::DFileInfo::AttributeExtendID, QVariant>>("QMap<dfmio::DFileInfo::AttributeExtendID, QVariant>");
}

void DFMBASE_NAMESPACE::AbstractFileInfo::setProxy(const AbstractFileInfoPointer &proxy)
{
    dptr->proxy = proxy;
}

bool dfmbase::AbstractFileInfo::hasProxy()
{
    return dptr->proxy != nullptr;
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
    dptr->url = fileinfo.dptr->url;
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
    return dptr->url == fileinfo.dptr->url;
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

bool DFMBASE_NAMESPACE::AbstractFileInfo::initQuerier()
{
    CALL_PROXY(initQuerier());

    return false;
}

void DFMBASE_NAMESPACE::AbstractFileInfo::initQuerierAsync(int ioPriority, DFMBASE_NAMESPACE::AbstractFileInfo::initQuerierAsyncCallback func, void *userData)
{
    CALL_PROXY(initQuerierAsync(ioPriority, func, userData));
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
    CALL_PROXY(exists());

    return false;
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
    CALL_PROXY(refresh());
}

void DFMBASE_NAMESPACE::AbstractFileInfo::refresh(DFileInfo::AttributeID id, const QVariant &value)
{
    CALL_PROXY(refresh(id, value));
}
/*!
  * \brief 获取文件名称，默认是获取文件的全名称带suffix，此接口不会实现异步，全部使用Qurl去
  * 处理或者字符串处理，这都比较快
  * \param FileNameInfoType
  */
QString dfmbase::AbstractFileInfo::nameInfo(const dfmbase::AbstractFileInfo::FileNameInfoType type) const
{
    CALL_PROXY(nameInfo(type));
    switch (type) {
    case FileNameInfoType::kFileName:
        [[fallthrough]];
    case FileNameInfoType::kFileNameOfRename:
        return fileName();
    case FileNameInfoType::kBaseName:
        [[fallthrough]];
    case FileNameInfoType::kBaseNameOfRename:
        return baseName();
    case FileNameInfoType::kCompleteBaseName:
        [[fallthrough]];
    case FileNameInfoType::kSuffix:
        [[fallthrough]];
    case FileNameInfoType::kCompleteSuffix:
        [[fallthrough]];
    case FileNameInfoType::kSuffixOfRename:
        [[fallthrough]];
    case FileNameInfoType::kFileCopyName:
        [[fallthrough]];
    case FileNameInfoType::kMimeTypeName:
        return QString();
    case FileNameInfoType::kIconName:
        return const_cast<AbstractFileInfo *>(this)->fileMimeType().iconName();
    case FileNameInfoType::kGenericIconName:
        return const_cast<AbstractFileInfo *>(this)->fileMimeType().genericIconName();
    default:
        return QString();
    }
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
    CALL_PROXY(filePath());
    return QString();
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
    CALL_PROXY(absoluteFilePath());
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
    QString filePath = this->filePath();

    if (filePath.endsWith(QDir::separator())) {
        filePath.chop(1);
    }

    int index = filePath.lastIndexOf(QDir::separator());

    if (index >= 0) {
        return filePath.mid(index + 1);
    }

    return filePath;
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
    const QString &fileName = this->fileName();
    const QString &suffix = this->nameInfo(FileNameInfoType::kSuffix);

    if (suffix.isEmpty()) {
        return fileName;
    }

    return fileName.left(fileName.length() - suffix.length() - 1);
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
    CALL_PROXY(path());

    return QString();
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
    CALL_PROXY(absolutePath());

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

    CALL_PROXY(canonicalPath());

    return filePath();
}
/*!
 * \brief url 获取文件的url
 *
 * \param
 *
 * \return QUrl 返回设置的url
 */
QUrl AbstractFileInfo::url() const
{
    return dptr->url;
}

bool DFMBASE_NAMESPACE::AbstractFileInfo::canRename() const
{

    CALL_PROXY(canRename());

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
bool AbstractFileInfo::isReadable() const
{
    CALL_PROXY(isReadable());

    return false;
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
    CALL_PROXY(isWritable());

    return false;
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
    CALL_PROXY(isExecutable());

    return false;
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
    CALL_PROXY(isHidden());

    return false;
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
    CALL_PROXY(isFile());

    return false;
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
    CALL_PROXY(isDir());

    return false;
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
    CALL_PROXY(isSymLink());

    return false;
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
    CALL_PROXY(isRoot());

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
    CALL_PROXY(isBundle());

    return false;
}

/*!
 * \brief inode linux系统下的唯一表示符
 *
 * \return quint64 文件的inode
 */
quint64 DFMBASE_NAMESPACE::AbstractFileInfo::inode() const
{
    CALL_PROXY(inode());

    return 0;
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
    CALL_PROXY(symLinkTarget());

    return QString();
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
    CALL_PROXY(owner());

    return QString();
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
    CALL_PROXY(ownerId());

    return static_cast<uint>(-1);
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
    CALL_PROXY(group());

    return QString();
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
    CALL_PROXY(groupId());

    return static_cast<uint>(-1);
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
    CALL_PROXY(permission(permissions));

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
    CALL_PROXY(permissions());

    return QFileDevice::Permissions();
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
    CALL_PROXY(size());

    return 0;
}

/*!
 * \brief DFMBASE_NAMESPACE::AbstractFileInfo::sizeFormat 使用kb，mb，gb显示文件大小
 * \return
 */
QString DFMBASE_NAMESPACE::AbstractFileInfo::sizeFormat() const
{
    CALL_PROXY(sizeFormat());

    return QString();
}

/*!
  * \brief 获取文件的时间信息
  * \param FileTimeType
  */
QVariant dfmbase::AbstractFileInfo::timeInfo(const dfmbase::AbstractFileInfo::FileTimeType type) const
{
    CALL_PROXY(timeInfo(type));

    switch (type) {
    case AbstractFileInfo::FileTimeType::kCreateTime:
        [[fallthrough]];
    case AbstractFileInfo::FileTimeType::kBirthTime:
        [[fallthrough]];
    case AbstractFileInfo::FileTimeType::kMetadataChangeTime:
        [[fallthrough]];
    case AbstractFileInfo::FileTimeType::kLastModified:
        [[fallthrough]];
    case AbstractFileInfo::FileTimeType::kLastRead:
        [[fallthrough]];
    case AbstractFileInfo::FileTimeType::kDeletionTime:
        return QDateTime();
    case AbstractFileInfo::FileTimeType::kCreateTimeSecond:
        [[fallthrough]];
    case AbstractFileInfo::FileTimeType::kBirthTimeSecond:
        [[fallthrough]];
    case AbstractFileInfo::FileTimeType::kMetadataChangeTimeSecond:
        [[fallthrough]];
    case AbstractFileInfo::FileTimeType::kLastModifiedSecond:
        [[fallthrough]];
    case AbstractFileInfo::FileTimeType::kLastReadSecond:
        [[fallthrough]];
    case AbstractFileInfo::FileTimeType::kDeletionTimeSecond:
        [[fallthrough]];
    case AbstractFileInfo::FileTimeType::kCreateTimeMSecond:
        [[fallthrough]];
    case AbstractFileInfo::FileTimeType::kBirthTimeMSecond:
        [[fallthrough]];
    case AbstractFileInfo::FileTimeType::kMetadataChangeTimeMSecond:
        [[fallthrough]];
    case AbstractFileInfo::FileTimeType::kLastModifiedMSecond:
        [[fallthrough]];
    case AbstractFileInfo::FileTimeType::kLastReadMSecond:
        [[fallthrough]];
    case AbstractFileInfo::FileTimeType::kDeletionTimeMSecond:
        return 0;
    default:
        return QVariant();
    }
}
/*!
 * \brief DFMBASE_NAMESPACE::AbstractFileInfo::countChildFile 获取目录下有多少个文件（只有下一级）
 * \return 返回文件数量
 */
int DFMBASE_NAMESPACE::AbstractFileInfo::countChildFile() const
{
    CALL_PROXY(countChildFile());

    return -1;
}

/*!
 * \brief DFMBASE_NAMESPACE::AbstractFileInfo::fileType 获取文件的设备类型
 * \return 返回文件的设备类型
 */
AbstractFileInfo::FileType DFMBASE_NAMESPACE::AbstractFileInfo::fileType() const
{
    CALL_PROXY(fileType());

    return FileType::kUnknown;
}

/*!
 * \brief DFMBASE_NAMESPACE::AbstractFileInfo::getUrlByChildFileName Get the URL based on the name of the sub file
 * \param fileName Sub file name
 * \return URL of the file
 */
QUrl DFMBASE_NAMESPACE::AbstractFileInfo::getUrlByChildFileName(const QString &fileName) const
{
    CALL_PROXY(getUrlByChildFileName(fileName));

    if (!isDir()) {
        return QUrl();
    }
    QUrl theUrl = url();
    theUrl.setPath(DFMIO::DFMUtils::buildFilePath(absoluteFilePath().toStdString().c_str(),
                                                  fileName.toStdString().c_str(), nullptr));
    return theUrl;
}
/*!
 * \brief DFMBASE_NAMESPACE::AbstractFileInfo::getUrlByNewFileName Get URL based on new file name
 * \param fileName New file name
 * \return URL of the file
 */
QUrl DFMBASE_NAMESPACE::AbstractFileInfo::getUrlByNewFileName(const QString &fileName) const
{
    CALL_PROXY(getUrlByNewFileName(fileName));

    QUrl theUrl = url();
    const QString &newPath = DFMIO::DFMUtils::buildFilePath(absolutePath().toStdString().c_str(), fileName.toStdString().c_str(), nullptr);
    theUrl.setPath(newPath);

    return theUrl;
}
/*!
 * \brief 获取文件路径，默认是文件全路径，此接口不会实现异步，全部使用Qurl去
 * 处理或者字符串处理，这都比较快
 * \param FileNameInfoType
 */
QString dfmbase::AbstractFileInfo::displayInfo(const AbstractFileInfo::DisplayInfoType type) const
{
    CALL_PROXY(displayInfo(type));
    switch (type) {
    case AbstractFileInfo::DisplayInfoType::kSizeDisplayName:
        if (isDir())
            return "-";   // for dir don't display items count, highly improve the view's performance
        else
            return FileUtils::formatSize(size());
    case AbstractFileInfo::DisplayInfoType::kFileDisplayPath:
        return dptr->url.path();
    case AbstractFileInfo::DisplayInfoType::kMimeTypeDisplayName:
        return MimeTypeDisplayManager::instance()->displayName(nameInfo(FileNameInfoType::kMimeTypeName));
    case AbstractFileInfo::DisplayInfoType::kFileTypeDisplayName:
        return QString::number(static_cast<int>(MimeTypeDisplayManager::
                                                        instance()
                                                                ->displayNameToEnum(const_cast<AbstractFileInfo *>(this)->fileMimeType().name())))
                .append(nameInfo(FileNameInfoType::kSuffix));
    default:
        return QString();
    }
}
/*!
 * \brief DFMBASE_NAMESPACE::AbstractFileInfo::canRedirectionFileUrl Can I redirect files
 * \return
 */
bool DFMBASE_NAMESPACE::AbstractFileInfo::canRedirectionFileUrl() const
{
    CALL_PROXY(canRedirectionFileUrl());

    return false;
}
/*!
 * \brief DFMBASE_NAMESPACE::AbstractFileInfo::redirectedFileUrl redirection file
 * \return
 */
QUrl DFMBASE_NAMESPACE::AbstractFileInfo::redirectedFileUrl() const
{
    CALL_PROXY(redirectedFileUrl());

    return url();
}

/*!
 * \brief DFMBASE_NAMESPACE::AbstractFileInfo::originalUrl original file
 * if file is trash, return original path
 * \return QUrl
 */
QUrl DFMBASE_NAMESPACE::AbstractFileInfo::originalUrl() const
{
    CALL_PROXY(originalUrl());

    return url();
}

bool DFMBASE_NAMESPACE::AbstractFileInfo::canMoveOrCopy() const
{
    CALL_PROXY(canMoveOrCopy());

    return true;
}

/*!
 * \brief DFMBASE_NAMESPACE::AbstractFileInfo::canDrop
 * \return
 */
bool DFMBASE_NAMESPACE::AbstractFileInfo::canDrop()
{
    if (isPrivate()) {
        return false;
    }

    if (!isSymLink()) {
        const bool isDesktop = nameInfo(FileNameInfoType::kMimeTypeName) == Global::Mime::kTypeAppXDesktop;
        return isDir() || isDesktop;
    }

    AbstractFileInfoPointer info = nullptr;
    QString linkTargetPath = symLinkTarget();

    do {
        const QUrl &targetUrl = QUrl::fromLocalFile(linkTargetPath);

        if (targetUrl == url()) {
            return false;
        }

        info = InfoFactory::create<AbstractFileInfo>(targetUrl);

        if (!info) {
            return false;
        }

        linkTargetPath = info->symLinkTarget();
    } while (info->isSymLink());

    return info->canDrop();
}

bool DFMBASE_NAMESPACE::AbstractFileInfo::canDrag()
{
    CALL_PROXY(canDrag());
    return true;
}

/*!
 * \brief DFMBASE_NAMESPACE::AbstractFileInfo::parentUrl
 * \return
 */
QUrl DFMBASE_NAMESPACE::AbstractFileInfo::parentUrl() const
{
    CALL_PROXY(parentUrl());

    return UrlRoute::urlParent(url());
}

bool DFMBASE_NAMESPACE::AbstractFileInfo::isAncestorsUrl(const QUrl &url, QList<QUrl> *ancestors) const
{
    CALL_PROXY(isAncestorsUrl(url, ancestors));

    QUrl parentUrl = this->parentUrl();

    forever {
        if (ancestors && parentUrl.isValid()) {
            ancestors->append(parentUrl);
        }

        if (UniversalUtils::urlEquals(parentUrl, url)) {
            return true;
        }

        auto fileInfo = InfoFactory::create<AbstractFileInfo>(parentUrl);

        if (!fileInfo) {
            break;
        }

        const QUrl &pu = fileInfo->parentUrl();

        if (pu == parentUrl) {
            break;
        }

        parentUrl = pu;
    }

    return false;
}
/*!
  * \brief view进入当前目录的提示信息，固定字符串处理，不实现异步
  * \param SupportType
  */
Qt::DropActions DFMBASE_NAMESPACE::AbstractFileInfo::supportedAttributes(const AbstractFileInfo::SupportType type) const
{
    CALL_PROXY(supportedAttributes(type));

    switch (type) {
    case SupportType::kDrag:
        return Qt::CopyAction | Qt::MoveAction | Qt::LinkAction;
    case SupportType::kDrop:
        if (isWritable()) {
            return Qt::CopyAction | Qt::MoveAction | Qt::LinkAction;
        }

        if (const_cast<AbstractFileInfo *>(this)->canDrop()) {
            return Qt::CopyAction | Qt::MoveAction;
        }
        return Qt::IgnoreAction;
    default:
        return Qt::IgnoreAction;
    }
}
/*!
 * \brief DFMBASE_NAMESPACE::AbstractFileInfo::canDragCompress
 * \return
 */
bool DFMBASE_NAMESPACE::AbstractFileInfo::canDragCompress() const
{
    CALL_PROXY(canDragCompress());

    return false;
}

bool DFMBASE_NAMESPACE::AbstractFileInfo::canFetch() const
{
    CALL_PROXY(canFetch());

    return isDir() && !isPrivate();
}

bool DFMBASE_NAMESPACE::AbstractFileInfo::canHidden() const
{
    CALL_PROXY(canHidden());
    return true;
}
/*!
 * \brief DFMBASE_NAMESPACE::AbstractFileInfo::isDragCompressFileFormat
 * \return
 */
bool DFMBASE_NAMESPACE::AbstractFileInfo::isDragCompressFileFormat() const
{
    CALL_PROXY(isDragCompressFileFormat());

    return false;
}

bool DFMBASE_NAMESPACE::AbstractFileInfo::isPrivate() const
{
    CALL_PROXY(isPrivate());

    return false;
}

bool DFMBASE_NAMESPACE::AbstractFileInfo::canDelete() const
{
    CALL_PROXY(canDelete());
    return false;
}

bool DFMBASE_NAMESPACE::AbstractFileInfo::canTrash() const
{
    CALL_PROXY(canTrash());
    return false;
}
/*!
 * \brief view进入当前目录的提示信息，固定字符串处理，不实现异步
 * \param ViewType
 */
QString DFMBASE_NAMESPACE::AbstractFileInfo::viewTip(const AbstractFileInfo::ViewType type) const
{
    switch (type) {
    case ViewType::kEmptyDir:
        return QObject::tr("Folder is empty");
    case ViewType::kLoading:
        return QObject::tr("Loading...");
    default:
        return QString();
    }
}

QVariant DFMBASE_NAMESPACE::AbstractFileInfo::customAttribute(const char *key, const DFileInfo::DFileAttributeType type)
{
    CALL_PROXY(customAttribute(key, type));

    return QVariant();
}

void DFMBASE_NAMESPACE::AbstractFileInfo::mediaInfoAttributes(DFileInfo::MediaType type, QList<DFileInfo::AttributeExtendID> ids) const
{
    CALL_PROXY(mediaInfoAttributes(type, ids));
}

bool DFMBASE_NAMESPACE::AbstractFileInfo::notifyAttributeChanged()
{
    CALL_PROXY(notifyAttributeChanged());

    return false;
}
/*!
  * \brief setExtendedAttributes 设置文件的扩展属性
  * \param AbstractFileInfo::FileExtendedInfoType 扩展属性key \param QVariant 属性
  * return
  */
void DFMBASE_NAMESPACE::AbstractFileInfo::setExtendedAttributes(const AbstractFileInfo::FileExtendedInfoType &key, const QVariant &value)
{
    CALL_PROXY(setExtendedAttributes(key, value));
}
/*!
 * \brief DFMBASE_NAMESPACE::AbstractFileInfo::fileIcon
 * \return
 */
QIcon DFMBASE_NAMESPACE::AbstractFileInfo::fileIcon()
{
    CALL_PROXY(fileIcon());

    return QIcon();
}

/*!
 * \brief DFMBASE_NAMESPACE::AbstractFileInfo::fileMimeType
 * \return
 */
QMimeType DFMBASE_NAMESPACE::AbstractFileInfo::fileMimeType(QMimeDatabase::MatchMode mode /*= QMimeDatabase::MatchDefault*/)
{
    CALL_PROXY(fileMimeType(mode));

    return QMimeType();
}

/*!
 * \brief 用于获取特定类型文件的特定属性扩展接口
 * \return 返回特定属性的hash表
 */
QVariantHash DFMBASE_NAMESPACE::AbstractFileInfo::extraProperties() const
{
    CALL_PROXY(extraProperties());

    return QVariantHash();
}

QVariant DFMBASE_NAMESPACE::AbstractFileInfo::customData(int role) const
{
    CALL_PROXY(customData(role));
    return QVariant();
}

/*!
 * \class DAbstractFileInfoPrivate 抽象文件信息私有类
 *
 * \brief 主要存储文件信息的成员变量和数据
 */
AbstractFileInfoPrivate::AbstractFileInfoPrivate(const QUrl &url, AbstractFileInfo *qq)
    : url(url), q(qq)
{
}

AbstractFileInfoPrivate::~AbstractFileInfoPrivate()
{
}
}
