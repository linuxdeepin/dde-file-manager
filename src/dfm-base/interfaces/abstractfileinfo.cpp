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
#include "dfm-base/utils/chinese2pinyin.h"
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
QString dfmbase::AbstractFileInfo::nameOf(const NameInfoType type) const
{
    CALL_PROXY(nameOf(type));
    switch (type) {
    case FileNameInfoType::kFileName:
        [[fallthrough]];
    case FileNameInfoType::kFileNameOfRename:
        return dptr->fileName();
    case FileNameInfoType::kBaseName:
        [[fallthrough]];
    case FileNameInfoType::kBaseNameOfRename:
        return dptr->baseName();
    case FileNameInfoType::kIconName:
        return const_cast<AbstractFileInfo *>(this)->fileMimeType().iconName();
    case FileNameInfoType::kGenericIconName:
        return const_cast<AbstractFileInfo *>(this)->fileMimeType().genericIconName();
    default:
        return QString();
    }
}
/*!
  * \brief 获取文件路径，默认是文件全路径，此接口不会实现异步，全部使用Qurl去
  * 处理或者字符串处理，这都比较快
  * \param FileNameInfoType
  */
QString dfmbase::AbstractFileInfo::pathOf(const PathInfoType type) const
{
    CALL_PROXY(pathOf(type));

    return QString();
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
  * \brief 获取文件的时间信息
  * \param FileTimeType
  */
QVariant dfmbase::AbstractFileInfo::timeOf(const TimeInfoType type) const
{
    CALL_PROXY(timeOf(type));

    switch (type) {
    case TimeInfoType::kCreateTime:
        [[fallthrough]];
    case TimeInfoType::kBirthTime:
        [[fallthrough]];
    case TimeInfoType::kMetadataChangeTime:
        [[fallthrough]];
    case TimeInfoType::kLastModified:
        [[fallthrough]];
    case TimeInfoType::kLastRead:
        [[fallthrough]];
    case TimeInfoType::kDeletionTime:
        return QDateTime();
    case TimeInfoType::kCreateTimeSecond:
        [[fallthrough]];
    case TimeInfoType::kBirthTimeSecond:
        [[fallthrough]];
    case TimeInfoType::kMetadataChangeTimeSecond:
        [[fallthrough]];
    case TimeInfoType::kLastModifiedSecond:
        [[fallthrough]];
    case TimeInfoType::kLastReadSecond:
        [[fallthrough]];
    case TimeInfoType::kDeletionTimeSecond:
        [[fallthrough]];
    case TimeInfoType::kCreateTimeMSecond:
        [[fallthrough]];
    case TimeInfoType::kBirthTimeMSecond:
        [[fallthrough]];
    case TimeInfoType::kMetadataChangeTimeMSecond:
        [[fallthrough]];
    case TimeInfoType::kLastModifiedMSecond:
        [[fallthrough]];
    case TimeInfoType::kLastReadMSecond:
        [[fallthrough]];
    case TimeInfoType::kDeletionTimeMSecond:
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

int dfmbase::AbstractFileInfo::countChildFileAsync() const
{
    CALL_PROXY(countChildFileAsync());

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
 * \brief 获取文件路径，默认是文件全路径，此接口不会实现异步，全部使用Qurl去
 * 处理或者字符串处理，这都比较快
 * \param FileNameInfoType
 */
QString dfmbase::AbstractFileInfo::displayOf(const DisPlayInfoType type) const
{
    CALL_PROXY(displayOf(type));
    switch (type) {
    case DisPlayInfoType::kSizeDisplayName:
        if (isAttributes(OptInfoType::kIsDir))
            return "-";   // for dir don't display items count, highly improve the view's performance
        else
            return FileUtils::formatSize(size());
    case DisPlayInfoType::kFileDisplayPath:
        return dptr->url.path();
    case DisPlayInfoType::kMimeTypeDisplayName:
        return MimeTypeDisplayManager::instance()->displayName(nameOf(FileNameInfoType::kMimeTypeName));
    case DisPlayInfoType::kFileTypeDisplayName:
        return QString::number(static_cast<int>(MimeTypeDisplayManager::
                                                        instance()
                                                                ->displayNameToEnum(const_cast<AbstractFileInfo *>(this)->fileMimeType().name())))
                .append(nameOf(FileNameInfoType::kSuffix));
    default:
        return QString();
    }
}

/*!
 * \brief 获取文件url，默认是文件的url，此接口不会实现异步，全部使用Qurl去
 * 处理或者字符串处理，这都比较快
 * \param FileUrlInfoType
 */
QUrl dfmbase::AbstractFileInfo::urlOf(const UrlInfoType type) const
{
    // FileUrlInfoType::kUrl don't you proxy,must use the original url
    if (FileUrlInfoType::kUrl == type)
        return dptr->url;
    CALL_PROXY(urlOf(type));
    switch (type) {
    case FileUrlInfoType::kOriginalUrl:
        [[fallthrough]];
    case FileUrlInfoType::kRedirectedFileUrl:
        return dptr->url;
    case FileUrlInfoType::kParentUrl:
        return UrlRoute::urlParent(dptr->url);
    default:
        return QUrl();
    }
}

QUrl dfmbase::AbstractFileInfo::getUrlByType(const UrlInfoType type, const QString &fileName) const
{
    CALL_PROXY(getUrlByType(type, fileName));
    switch (type) {
    case FileUrlInfoType::kGetUrlByNewFileName:
        return dptr->getUrlByNewFileName(fileName);
    case FileUrlInfoType::kGetUrlByChildFileName:
        return dptr->getUrlByChildFileName(fileName);
    default:
        return QUrl();
    }
}
/*!
  * \brief view进入当前目录的提示信息，固定字符串处理，不实现异步
  * \param SupportType
  */
Qt::DropActions DFMBASE_NAMESPACE::AbstractFileInfo::supportedOfAttributes(const SupportedType type) const
{
    CALL_PROXY(supportedOfAttributes(type));

    switch (type) {
    case SupportType::kDrag:
        return Qt::CopyAction | Qt::MoveAction | Qt::LinkAction;
    case SupportType::kDrop:
        if (isAttributes(OptInfoType::kIsWritable)) {
            return Qt::CopyAction | Qt::MoveAction | Qt::LinkAction;
        }

        if (dptr->canDrop()) {
            return Qt::CopyAction | Qt::MoveAction;
        }
        return Qt::IgnoreAction;
    default:
        return Qt::IgnoreAction;
    }
}

/*!
 * \brief 获取文件扩展属性，如果创建时使用的异步，那么这里获取就是使用异步，没获取到就是
 * 默认，获取了就是读取属性
 * \param FileIsType
 */
bool dfmbase::AbstractFileInfo::isAttributes(const OptInfoType type) const
{
    CALL_PROXY(isAttributes(type));
    switch (type) {
    case FileIsType::kIsRoot:
        return pathOf(PathInfoType::kFilePath) == "/";
    default:
        return false;
    }
}

bool dfmbase::AbstractFileInfo::canAttributes(const CanableInfoType type) const
{
    CALL_PROXY(canAttributes(type));
    switch (type) {
    case FileCanType::kCanFetch:
        return isAttributes(OptInfoType::kIsDir)
                && !isAttributes(OptInfoType::kIsPrivate);
    case FileCanType::kCanDrop:
        return dptr->canDrop();
    case FileCanType::kCanDrag:
        [[fallthrough]];
    case FileCanType::kCanHidden:
        [[fallthrough]];
    case FileCanType::kCanMoveOrCopy:
        return true;
    default:
        return false;
    }
}

QVariant dfmbase::AbstractFileInfo::extendAttributes(const ExtInfoType type) const
{
    CALL_PROXY(extendAttributes(type));
    switch (type) {
    case FileExtendedInfoType::kInode:
        return 0;
    case FileExtendedInfoType::kOwner:
        [[fallthrough]];
    case FileExtendedInfoType::kSizeFormat:
        [[fallthrough]];
    case FileExtendedInfoType::kGroup:
        return QString();
    case FileExtendedInfoType::kOwnerId:
        [[fallthrough]];
    case FileExtendedInfoType::kGroupId:
        return static_cast<uint>(-1);
    default:
        return dptr->extendOtherCache.value(type);
    }
}

/*!
 * \brief view进入当前目录的提示信息，固定字符串处理，不实现异步
 * \param ViewType
 */
QString DFMBASE_NAMESPACE::AbstractFileInfo::viewOfTip(const ViewInfoType type) const
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

QMap<DFMIO::DFileInfo::AttributeExtendID, QVariant> DFMBASE_NAMESPACE::AbstractFileInfo::mediaInfoAttributes(DFileInfo::MediaType type, QList<DFileInfo::AttributeExtendID> ids) const
{
    CALL_PROXY(mediaInfoAttributes(type, ids));
    return QMap<DFMIO::DFileInfo::AttributeExtendID, QVariant>();
}

/*!
  * \brief setExtendedAttributes 设置文件的扩展属性
  * \param ExInfo 扩展属性key \param QVariant 属性
  * return
  */
void DFMBASE_NAMESPACE::AbstractFileInfo::setExtendedAttributes(const ExtInfoType &key, const QVariant &value)
{
    CALL_PROXY(setExtendedAttributes(key, value));
    dptr->extendOtherCache.insert(key, value);
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

QMimeType dfmbase::AbstractFileInfo::fileMimeTypeAsync(QMimeDatabase::MatchMode mode)
{
    CALL_PROXY(fileMimeTypeAsync(mode));

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

/*!
 * \brief DFMBASE_NAMESPACE::AbstractFileInfo::getUrlByChildFileName Get the URL based on the name of the sub file
 * \param fileName Sub file name
 * \return URL of the file
 */
QUrl DFMBASE_NAMESPACE::AbstractFileInfoPrivate::getUrlByChildFileName(const QString &fileName) const
{
    if (!q->isAttributes(OptInfoType::kIsDir)) {
        return QUrl();
    }
    QUrl theUrl = url;
    theUrl.setPath(DFMIO::DFMUtils::buildFilePath(q->pathOf(PathInfoType::kAbsoluteFilePath).toStdString().c_str(),
                                                  fileName.toStdString().c_str(), nullptr));
    return theUrl;
}

/*!
 * \brief DFMBASE_NAMESPACE::AbstractFileInfo::getUrlByNewFileName Get URL based on new file name
 * \param fileName New file name
 * \return URL of the file
 */
QUrl DFMBASE_NAMESPACE::AbstractFileInfoPrivate::getUrlByNewFileName(const QString &fileName) const
{
    QUrl theUrl = url;
    const QString &newPath = DFMIO::DFMUtils::buildFilePath(q->pathOf(PathInfoType::kAbsolutePath).toStdString().c_str(), fileName.toStdString().c_str(), nullptr);
    theUrl.setPath(newPath);

    return theUrl;
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
QString AbstractFileInfoPrivate::fileName() const
{
    QString filePath = q->pathOf(PathInfoType::kFilePath);

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
QString AbstractFileInfoPrivate::baseName() const
{
    const QString &fileName = this->fileName();
    const QString &suffix = q->nameOf(NameInfoType::kSuffix);

    if (suffix.isEmpty()) {
        return fileName;
    }

    return fileName.left(fileName.length() - suffix.length() - 1);
}

/*!
 * \brief DFMBASE_NAMESPACE::AbstractFileInfo::canDrop
 * \return
 */
bool DFMBASE_NAMESPACE::AbstractFileInfoPrivate::canDrop()
{
    if (q->isAttributes(OptInfoType::kIsPrivate)) {
        return false;
    }

    if (!q->isAttributes(OptInfoType::kIsSymLink)) {
        const bool isDesktop = q->nameOf(NameInfoType::kMimeTypeName) == Global::Mime::kTypeAppXDesktop;
        return q->isAttributes(OptInfoType::kIsDir) || isDesktop;
    }

    AbstractFileInfoPointer info = nullptr;
    QString linkTargetPath = q->pathOf(PathInfoType::kSymLinkTarget);

    do {
        const QUrl &targetUrl = QUrl::fromLocalFile(linkTargetPath);

        if (targetUrl == url) {
            return false;
        }

        info = InfoFactory::create<AbstractFileInfo>(targetUrl);

        if (!info) {
            return false;
        }

        linkTargetPath = info->pathOf(PathInfoType::kSymLinkTarget);
    } while (info->isAttributes(OptInfoType::kIsSymLink));

    return info->canAttributes(CanableInfoType::kCanDrop);
}

}
