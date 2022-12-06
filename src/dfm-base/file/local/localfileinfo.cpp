/* * Copyright (C) 2021 ~ 2021 Uniontech Software Technology Co., Ltd
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

#include "dfm-base/base/urlroute.h"
#include "dfm-base/base/standardpaths.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/utils/chinese2pinyin.h"
#include "dfm-base/utils/sysinfoutils.h"
#include "dfm-base/utils/decorator/decoratorfileenumerator.h"
#include "dfm-base/file/local/localfileiconprovider.h"
#include "dfm-base/mimetype/dmimedatabase.h"
#include "dfm-base/mimetype/mimetypedisplaymanager.h"
#include "dfm-base/base/application/application.h"

#include <dfm-io/dfmio_utils.h>
#include <dfm-io/local/dlocalfileinfo.h>

#include <QDateTime>
#include <QDir>
#include <QMap>
#include <QPainter>
#include <QApplication>
#include <QtConcurrent>
#include <qplatformdefs.h>

#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <mntent.h>

static constexpr uint16_t kRequestThumbnailDealy { 500 };

/*!
 * \class LocalFileInfo 本地文件信息类
 * \brief 内部实现本地文件的fileinfo，对应url的scheme是file://
 */
namespace dfmbase {

LocalFileInfo::LocalFileInfo(const QUrl &url)
    : AbstractFileInfo(url), d(new LocalFileInfoPrivate(url, this))
{
    dptr.reset(d);
    init(url);
}

LocalFileInfo::LocalFileInfo(const QUrl &url, QSharedPointer<DFileInfo> dfileInfo)
    : AbstractFileInfo(url), d(new LocalFileInfoPrivate(url, this))
{
    dptr.reset(d);
    init(url, dfileInfo);
}

LocalFileInfo::~LocalFileInfo()
{
    d = nullptr;
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

bool LocalFileInfo::initQuerier()
{
    if (d->dfmFileInfo)
        return d->dfmFileInfo->initQuerier();
    return false;
}

void LocalFileInfo::initQuerierAsync(int ioPriority, AbstractFileInfo::initQuerierAsyncCallback func, void *userData)
{
    if (d->dfmFileInfo)
        d->dfmFileInfo->initQuerierAsync(ioPriority, func, userData);
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
    bool exists = false;
    if (d->dfmFileInfo) {
        QReadLocker locker(&d->lock);
        exists = d->dfmFileInfo->exists();
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
    d->dfmFileInfo->refresh();
    d->loadingThumbnail = false;
    d->fileType = MimeDatabase::FileType::kUnknown;
    d->mimeTypeMode = QMimeDatabase::MatchMode::MatchDefault;
    d->enableThumbnail = false;
    d->extraProperties.clear();
    d->attributesExtend.clear();
    d->extendIDs.clear();
    d->isLocalDevice = QVariant();
    d->isCdRomDevice = QVariant();
    d->mimeType = QMimeType();
    d->mimeTypeMode = QMimeDatabase::MatchDefault;
    d->clearIcon();
}

void LocalFileInfo::refresh(DFileInfo::AttributeID id, const QVariant &value)
{
    QWriteLocker locker(&d->lock);
    if (value.isValid())
        d->dfmFileInfo->setAttribute(id, value);
}

QString LocalFileInfo::nameInfo(const AbstractFileInfo::FileNameInfoType type) const
{
    switch (type) {
    case FileNameInfoType::kFileName:
        return d->fileName();
    case FileNameInfoType::kBaseName:
        return d->baseName();
    case FileNameInfoType::kCompleteBaseName:
        return d->completeBaseName();
    case FileNameInfoType::kSuffix:
        return d->suffix();
    case FileNameInfoType::kCompleteSuffix:
        return const_cast<LocalFileInfoPrivate *>(d)->completeSuffix();
    case FileNameInfoType::kFileCopyName:
        return d->fileDisplayName();
    case FileNameInfoType::kIconName:
        return const_cast<LocalFileInfoPrivate *>(d)->iconName();
    case FileNameInfoType::kGenericIconName:
        return const_cast<LocalFileInfo *>(this)->fileMimeType().genericIconName();
    case FileNameInfoType::kMimeTypeName:
        return const_cast<LocalFileInfoPrivate *>(d)->mimeTypeName();
    default:
        return AbstractFileInfo::nameInfo(type);
    }
}
/*!
  * \brief 获取文件路径，默认是文件全路径，此接口不会实现异步，全部使用Qurl去
  * 处理或者字符串处理，这都比较快
  * \param FileNameInfoType
  */
QString LocalFileInfo::pathInfo(const dfmbase::AbstractFileInfo::FilePathInfoType type) const
{
    switch (type) {
    case FilePathInfoType::kFilePath:
        [[fallthrough]];
    case FilePathInfoType::kAbsoluteFilePath:
        [[fallthrough]];
    case FilePathInfoType::kCanonicalPath:
        return d->filePath();
    case FilePathInfoType::kPath:
        [[fallthrough]];
    case FilePathInfoType::kAbsolutePath:
        return d->path();
    case FilePathInfoType::kSymLinkTarget:
        return d->symLinkTarget();
    default:
        return AbstractFileInfo::pathInfo(type);
    }
}
/*!
 * \brief 获取文件url，默认是文件的url，此接口不会实现异步，全部使用Qurl去
 * 处理或者字符串处理，这都比较快
 * \param FileUrlInfoType
 */
QUrl LocalFileInfo::urlInfo(const AbstractFileInfo::FileUrlInfoType type) const
{
    switch (type) {
    case FileUrlInfoType::kRedirectedFileUrl:
        return d->redirectedFileUrl();
    default:
        return AbstractFileInfo::urlInfo(type);
    }
}

bool LocalFileInfo::isAttributes(const AbstractFileInfo::FileIsType type) const
{
    switch (type) {
    case FileIsType::kIsFile:
        [[fallthrough]];
    case FileIsType::kIsDir:
        [[fallthrough]];
    case FileIsType::kIsReadable:
        [[fallthrough]];
    case FileIsType::kIsWritable:
        [[fallthrough]];
    case FileIsType::kIsHidden:
        [[fallthrough]];
    case FileIsType::kIsSymLink:
        if (d->dfmFileInfo) {
            QReadLocker locker(&d->lock);
            return d->dfmFileInfo->attribute(d->getAttributeIDIsVector()[static_cast<int>(type)], nullptr).toBool();
        }
        return false;
    case FileIsType::kIsExecutable:
        return d->isExecutable();
    case FileIsType::kIsRoot:
        return d->filePath() == "/";
    case FileIsType::kIsBundle:
        return QFileInfo(d->url.path()).isBundle();
    case FileIsType::kIsDragCompressFileFormat: {
        const QString &&name = d->fileName();
        return name.endsWith(".zip")
                || (name.endsWith(".7z")
                    && !name.endsWith(".tar.7z"));
    }
    case FileIsType::kIsPrivate:
        return d->isPrivate();
    default:
        return AbstractFileInfo::isAttributes(type);
    }
}

bool LocalFileInfo::canAttributes(const AbstractFileInfo::FileCanType type) const
{
    switch (type) {
    case FileCanType::kCanDelete:
        return d->canDelete();
    case FileCanType::kCanTrash:
        return d->canTrash();
    case FileCanType::kCanRename:
        return d->canRename();
    case FileCanType::kCanDragCompress:
        return isAttributes(AbstractFileInfo::FileIsType::kIsDragCompressFileFormat)
                && isAttributes(FileIsType::kIsReadable)
                && isAttributes(FileIsType::kIsWritable)
                && !FileUtils::isGvfsFile(dptr->url);
    case FileCanType::kCanHidden:
        if (FileUtils::isGphotoFile(d->url))
            return false;
        return true;
    default:
        return AbstractFileInfo::canAttributes(type);
    }
}

QVariant LocalFileInfo::extendedAttributes(const AbstractFileInfo::FileExtendedInfoType type) const
{
    switch (type) {
    case FileExtendedInfoType::kInode:
        [[fallthrough]];
    case FileExtendedInfoType::kOwner:
        [[fallthrough]];
    case FileExtendedInfoType::kGroup:
        [[fallthrough]];
    case FileExtendedInfoType::kOwnerId:
        [[fallthrough]];
    case FileExtendedInfoType::kGroupId:
        if (d->dfmFileInfo) {
            QReadLocker locker(&d->lock);
            return d->dfmFileInfo->attribute(d->getAttributeIDExtendVector()[static_cast<int>(type)], nullptr);
        }
        return QVariant();
    case FileExtendedInfoType::kFileLocalDevice:
        return d->isLocalDevice;
    case FileExtendedInfoType::kFileCdRomDevice:
        return d->isCdRomDevice;
    case FileExtendedInfoType::kFileIsHid:
        if (d->dfmFileInfo)
            return d->dfmFileInfo->attribute(DFileInfo::AttributeID::kStandardIsHidden, nullptr);
        return QVariant();
    case FileExtendedInfoType::kSizeFormat:
        return d->sizeFormat();
    default:
        return AbstractFileInfo::extendedAttributes(type);
    }
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
    QFileDevice::Permissions ps;

    if (d->dfmFileInfo) {
        QReadLocker locker(&d->lock);
        ps = static_cast<QFileDevice::Permissions>(static_cast<uint16_t>(d->dfmFileInfo->permissions()));
    }

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
    qint64 size = 0;
    if (d->dfmFileInfo) {
        QReadLocker locker(&d->lock);
        size = d->dfmFileInfo->attribute(DFileInfo::AttributeID::kStandardSize, nullptr).value<qint64>();
    }
    return size;
}
/*!
 * \brief timeInfo 获取文件的时间信息
 *
 * \return QVariant 普通的返回QDateTime, second和msecond返回qint64
 */
QVariant LocalFileInfo::timeInfo(const AbstractFileInfo::FileTimeType type) const
{
    qint64 data { 0 };
    if (type < FileTimeType::kDeletionTimeMSecond) {
        QReadLocker rlk(&d->lock);
        if (d->dfmFileInfo)
            data = d->dfmFileInfo->attribute(d->getAttributeIDVector()[static_cast<int>(type)], nullptr).value<qint64>();
    }
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
        return QDateTime::fromMSecsSinceEpoch(data);
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
    case AbstractFileInfo::FileTimeType::kCreateTimeMSecond:
        [[fallthrough]];
    case AbstractFileInfo::FileTimeType::kBirthTimeMSecond:
        [[fallthrough]];
    case AbstractFileInfo::FileTimeType::kMetadataChangeTimeMSecond:
        [[fallthrough]];
    case AbstractFileInfo::FileTimeType::kLastModifiedMSecond:
        [[fallthrough]];
    case AbstractFileInfo::FileTimeType::kLastReadMSecond:
        return data;
    default:
        return QVariant();
    }
}
/*!
 * \brief isBlockDev 获取是否是块设备
 *
 * \return bool 是否是块设备
 */
bool LocalFileInfo::isBlockDev() const
{
    return fileType() == FileType::kBlockDevice;
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
 * \brief fileType 获取文件类型
 *
 * \return DMimeDatabase::FileType 文件设备类型
 */
LocalFileInfo::FileType LocalFileInfo::fileType() const
{
    FileType fileType { FileType::kUnknown };
    {
        QReadLocker locker(&d->lock);
        if (d->fileType != MimeDatabase::FileType::kUnknown) {
            fileType = FileType(d->fileType);
            return fileType;
        }
    }

    const QUrl &fileUrl = d->url;
    if (FileUtils::isTrashFile(fileUrl) && isAttributes(FileIsType::kIsSymLink)) {
        {
            QWriteLocker locker(&d->lock);
            d->fileType = MimeDatabase::FileType::kRegularFile;
        }
        fileType = FileType(MimeDatabase::FileType::kRegularFile);
        return fileType;
    }

    // Cannot access statBuf.st_mode from the filesystem engine, so we have to stat again.
    // In addition we want to follow symlinks.
    const QString &absoluteFilePath = d->filePath();
    const QByteArray &nativeFilePath = QFile::encodeName(absoluteFilePath);
    QT_STATBUF statBuffer;
    if (QT_STAT(nativeFilePath.constData(), &statBuffer) == 0) {
        if (S_ISDIR(statBuffer.st_mode))
            fileType = FileType(MimeDatabase::FileType::kDirectory);
        else if (S_ISCHR(statBuffer.st_mode))
            fileType = FileType(MimeDatabase::FileType::kCharDevice);
        else if (S_ISBLK(statBuffer.st_mode))
            fileType = FileType(MimeDatabase::FileType::kBlockDevice);
        else if (S_ISFIFO(statBuffer.st_mode))
            fileType = FileType(MimeDatabase::FileType::kFIFOFile);
        else if (S_ISSOCK(statBuffer.st_mode))
            fileType = FileType(MimeDatabase::FileType::kSocketFile);
        else if (S_ISREG(statBuffer.st_mode))
            fileType = FileType(MimeDatabase::FileType::kRegularFile);

        QWriteLocker locker(&d->lock);
        d->fileType = MimeDatabase::FileType(fileType);
    }
    return fileType;
}
/*!
 * \brief countChildFile 文件夹下子文件的个数，只统计下一层不递归
 *
 * \return int 子文件个数
 */
int LocalFileInfo::countChildFile() const
{
    if (isAttributes(FileIsType::kIsDir)) {
        const QString &path = d->filePath();
        QReadLocker locker(&d->lock);
        DecoratorFileEnumerator enumerator(path);
        return int(enumerator.fileCount());
    }

    return -1;
}

QString LocalFileInfo::displayInfo(const AbstractFileInfo::DisplayInfoType type) const
{
    if (type == AbstractFileInfo::DisplayInfoType::kFileDisplayName)
        return d->fileDisplayName();
    return AbstractFileInfo::displayInfo(type);
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

QIcon LocalFileInfo::fileIcon()
{
    const QUrl &fileUrl = d->url;

    if (FileUtils::containsCopyingFileUrl(fileUrl))
        return LocalFileIconProvider::globalProvider()->icon(this);

#ifdef DFM_MINIMUM
    d->enableThumbnail = 0;
#else
    if (d->enableThumbnail < 0) {
        bool isLocalDevice = false;
        bool isCdRomDevice = false;
        if (d->isLocalDevice.isValid())
            isLocalDevice = d->isLocalDevice.toBool();
        else
            isLocalDevice = FileUtils::isLocalDevice(fileUrl);
        if (d->isCdRomDevice.isValid())
            isCdRomDevice = d->isCdRomDevice.toBool();
        else
            isCdRomDevice = FileUtils::isCdRomDevice(fileUrl);

        d->enableThumbnail = isLocalDevice && !isCdRomDevice;
    }
#endif

    bool hasThumbnail = false;
    const int checkFast = ThumbnailProvider::instance()->hasThumbnailFast(d->mimeTypeName());
    if (1 == checkFast)
        hasThumbnail = true;
    else if (0 == checkFast)
        hasThumbnail = false;
    else
        hasThumbnail = ThumbnailProvider::instance()->hasThumbnail(fileMimeType());

    bool thumbEnabled = (d->enableThumbnail > 0) && hasThumbnail;
    return thumbEnabled ? d->thumbIcon() : d->defaultIcon();
}

QMimeType LocalFileInfo::fileMimeType(QMimeDatabase::MatchMode mode /*= QMimeDatabase::MatchDefault*/)
{
    const QUrl &url = d->url;
    QMimeType type;
    QMimeDatabase::MatchMode modeCache { QMimeDatabase::MatchMode::MatchDefault };
    {
        QReadLocker locker(&d->lock);
        type = d->mimeType;
        modeCache = d->mimeTypeMode;
    }

    if (!type.isValid() || modeCache != mode) {
        type = this->mimeType(url.path(), mode);
        QWriteLocker locker(&d->lock);
        d->mimeType = type;
        d->mimeTypeMode = mode;
    }

    return type;
}

QString LocalFileInfo::viewTip(const ViewType type) const
{
    if (type == ViewType::kEmptyDir) {
        if (!exists()) {
            return QObject::tr("File has been moved or deleted");
        } else if (!isAttributes(FileIsType::kIsReadable)) {
            return QObject::tr("You do not have permission to access this folder");
        } else if (isAttributes(FileIsType::kIsDir)) {
            if (!d->isExecutable())
                return QObject::tr("You do not have permission to traverse files in it");
        }
    }

    return AbstractFileInfo::viewTip(type);
}

QVariant LocalFileInfo::customAttribute(const char *key, const DFileInfo::DFileAttributeType type)
{
    if (d->dfmFileInfo) {
        QReadLocker locker(&d->lock);
        return d->dfmFileInfo->customAttribute(key, type);
    }
    return QVariant();
}

void LocalFileInfo::mediaInfoAttributes(DFileInfo::MediaType type, QList<DFileInfo::AttributeExtendID> ids) const
{
    if (d->dfmFileInfo) {
        d->extendIDs = ids;

        auto it = ids.begin();
        while (it != ids.end()) {
            if (d->attributesExtend.count(*it))
                it = ids.erase(it);
            else
                ++it;
        }

        if (!ids.isEmpty()) {
            QReadLocker locker(&d->lock);
            d->dfmFileInfo->attributeExtend(type, ids, std::bind(&LocalFileInfoPrivate::attributesExtendCallback, d, std::placeholders::_1, std::placeholders::_2));
        } else {
            d->attributesExtendCallback(true, {});
        }
    }
}

void LocalFileInfo::setExtendedAttributes(const FileExtendedInfoType &key, const QVariant &value)
{
    QWriteLocker locker(&d->lock);
    switch (key) {
    case FileExtendedInfoType::kFileLocalDevice:
        d->isLocalDevice = value;
        break;
    case FileExtendedInfoType::kFileCdRomDevice:
        d->isCdRomDevice = value;
        break;
    case FileExtendedInfoType::kFileIsHid:
        if (d->dfmFileInfo)
            d->dfmFileInfo->setAttribute(DFileInfo::AttributeID::kStandardIsHidden, value);
        break;
    default:
        break;
    }
}

void LocalFileInfo::init(const QUrl &url, QSharedPointer<DFMIO::DFileInfo> dfileInfo)
{
    d->mimeTypeMode = QMimeDatabase::MatchDefault;
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

    if (dfileInfo) {
        d->dfmFileInfo = dfileInfo;
        return;
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

QMimeType LocalFileInfo::mimeType(const QString &filePath, QMimeDatabase::MatchMode mode, const QString inod, const bool isGvfs)
{
    static DFMBASE_NAMESPACE::DMimeDatabase db;
    if (isGvfs) {
        return db.mimeTypeForFile(filePath, mode, inod, isGvfs);
    }
    return db.mimeTypeForFile(this->sharedFromThis(), mode);
}

QIcon LocalFileInfoPrivate::thumbIcon()
{
    QIcon icon;
    {   // if already loaded thumb just return it.
        QReadLocker rlk(&iconLock);
        icon = icons.value(IconType::kThumbIcon);
    }
    if (!icon.isNull())
        return icon;

    icon = QIcon(ThumbnailProvider::instance()->thumbnailFilePath(url, ThumbnailProvider::kLarge));
    if (!icon.isNull()) {
        QPixmap pixmap = icon.pixmap(ThumbnailProvider::kLarge, ThumbnailProvider::kLarge);
        QPainter pa(&pixmap);
        pa.setPen(Qt::gray);
        pa.drawPixmap(0, 0, pixmap);

        QIcon fileIcon;
        fileIcon.addPixmap(pixmap);
        {
            QWriteLocker wlk(&iconLock);
            icons.insert(IconType::kThumbIcon, fileIcon);
        }
        return fileIcon;
    }

    // else load thumb from DThumbnailProvider in async.
    // and before thumb thread finish, return default icon.
    if (!loadingThumbnail) {
        loadingThumbnail = true;
        if (!getIconFuture) {
            auto thumburl = url;
            getIconFuture.reset(new ThumbnailProvider::ThumbNailCreateFuture());
            QTimer::singleShot(kRequestThumbnailDealy, [=] {
                ThumbnailProvider::instance()->appendToProduceQueue(thumburl, ThumbnailProvider::kLarge, getIconFuture);
            });
        }
    } else if (getIconFuture && getIconFuture->finished) {
        onRequestThumbFinished(getIconFuture->thumbPath);
        icon = icons.value(IconType::kThumbIcon);
        return icon;
    }

    return defaultIcon();
}

QIcon LocalFileInfoPrivate::defaultIcon()
{
    QIcon icon;
    {
        QReadLocker rlk(&iconLock);
        icon = icons.value(LocalFileInfoPrivate::kDefaultIcon);
    }

    if (!icon.isNull())
        return icon;

    icon = LocalFileIconProvider::globalProvider()->icon(q);
    if (q->isAttributes(AbstractFileInfo::FileIsType::kIsSymLink)) {
        const auto &&target = symLinkTarget();
        if (target != filePath()) {
            AbstractFileInfoPointer info = InfoFactory::create<AbstractFileInfo>(QUrl::fromLocalFile(target));
            if (info)
                icon = info->fileIcon();
        }
    }

    {
        QWriteLocker wlk(&iconLock);
        icons.insert(LocalFileInfoPrivate::kDefaultIcon, icon);
    }

    return icon;
}

void LocalFileInfoPrivate::onRequestThumbFinished(const QString &path)
{
    if (path.isEmpty()) {
        // cannot generate thumbnail, using default icon
        auto icon = defaultIcon();
        QWriteLocker wlk(&iconLock);
        icons.insert(LocalFileInfoPrivate::kThumbIcon, icon);
    } else {
        thumbIcon();   // load icon from DThumbnailProvider
    }
    QWriteLocker wlk(&iconLock);
    loadingThumbnail = false;
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
QString LocalFileInfoPrivate::fileName() const
{
    QString fileName;
    if (dfmFileInfo) {
        QReadLocker locker(&const_cast<LocalFileInfoPrivate *>(this)->lock);
        fileName = dfmFileInfo->attribute(DFileInfo::AttributeID::kStandardName, nullptr).toString();
        // trans "/" to "smb-share:server=xxx,share=xxx"
        if (fileName == R"(/)" && FileUtils::isGvfsFile(url)) {
            fileName = dfmFileInfo->attribute(DFileInfo::AttributeID::kIdFilesystem, nullptr).toString();
        }
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
QString LocalFileInfoPrivate::baseName() const
{
    QString baseName;
    if (dfmFileInfo) {
        QReadLocker locker(&const_cast<LocalFileInfoPrivate *>(this)->lock);
        baseName = dfmFileInfo->attribute(DFileInfo::AttributeID::kStandardBaseName, nullptr).toString();
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
QString LocalFileInfoPrivate::completeBaseName() const
{
    QString completeBaseName;
    if (dfmFileInfo) {
        QReadLocker locker(&const_cast<LocalFileInfoPrivate *>(this)->lock);
        completeBaseName = dfmFileInfo->attribute(DFileInfo::AttributeID::kStandardCompleteBaseName, nullptr).toString();
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
QString LocalFileInfoPrivate::suffix() const
{
    QString suffix;
    if (dfmFileInfo) {
        QReadLocker locker(&const_cast<LocalFileInfoPrivate *>(this)->lock);
        suffix = dfmFileInfo->attribute(DFileInfo::AttributeID::kStandardSuffix, nullptr).toString();
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
QString LocalFileInfoPrivate::completeSuffix()
{
    QString completeSuffix;
    if (dfmFileInfo) {
        QReadLocker locker(&lock);
        completeSuffix = dfmFileInfo->attribute(DFileInfo::AttributeID::kStandardCompleteSuffix, nullptr).toString();
    }

    return completeSuffix;
}

QString LocalFileInfoPrivate::iconName()
{
    QString iconNameValue;
    if (SystemPathUtil::instance()->isSystemPath(filePath()))
        iconNameValue = SystemPathUtil::instance()->systemPathIconNameByPath(filePath());
    if (iconNameValue.isEmpty()) {
        if (dfmFileInfo) {
            QReadLocker locker(&lock);
            const QStringList &list = dfmFileInfo->attribute(DFileInfo::AttributeID::kStandardIcon, nullptr).toStringList();
            if (!list.isEmpty())
                iconNameValue = list.first();
        }
    }

    return iconNameValue;
}

QString LocalFileInfoPrivate::mimeTypeName()
{
    QString type;
    if (dfmFileInfo) {
        QReadLocker locker(&lock);
        type = dfmFileInfo->attribute(DFileInfo::AttributeID::kStandardContentType, nullptr).toString();
    }

    return type;
}

/*!
 * \brief fileDisplayName 文件的显示名称，一般为文件的名称
 *
 * \return QString 文件的显示名称
 */
QString LocalFileInfoPrivate::fileDisplayName() const
{
    QString &&path { filePath() };
    if (SystemPathUtil::instance()->isSystemPath(path)) {
        QString displayName { SystemPathUtil::instance()->systemPathDisplayNameByPath(path) };
        if (!displayName.isEmpty())
            return displayName;
    }

    QString fileDisplayName;
    if (dfmFileInfo) {
        QReadLocker locker(&const_cast<LocalFileInfoPrivate *>(this)->lock);
        fileDisplayName = dfmFileInfo->attribute(DFileInfo::AttributeID::kStandardDisplayName, nullptr).toString();

        // trans "/" to "smb-share:server=xxx,share=xxx"
        if (fileDisplayName == R"(/)" && FileUtils::isGvfsFile(url)) {
            fileDisplayName = dfmFileInfo->attribute(DFileInfo::AttributeID::kIdFilesystem, nullptr).toString();
        }
    }

    return fileDisplayName;
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
QString LocalFileInfoPrivate::path() const
{
    QString path;
    if (dfmFileInfo) {
        QReadLocker locker(&const_cast<LocalFileInfoPrivate *>(this)->lock);
        path = dfmFileInfo->attribute(DFileInfo::AttributeID::kStandardParentPath, nullptr).toString();
    }

    return path;
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
QString LocalFileInfoPrivate::filePath() const
{
    QString filePath;
    if (dfmFileInfo) {
        QReadLocker locker(&const_cast<LocalFileInfoPrivate *>(this)->lock);
        filePath = dfmFileInfo->attribute(DFileInfo::AttributeID::kStandardFilePath, nullptr).toString();
    }

    return filePath;
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
QString LocalFileInfoPrivate::symLinkTarget() const
{
    QString symLinkTarget;

    if (dfmFileInfo) {
        QReadLocker locker(&const_cast<LocalFileInfoPrivate *>(this)->lock);
        symLinkTarget = dfmFileInfo->attribute(DFileInfo::AttributeID::kStandardSymlinkTarget, nullptr).toString();
    }
    // the link target may be a relative path.
    if (!symLinkTarget.startsWith("/")) {
        auto currPath = path();
        if (currPath.right(1) != "/")
            currPath += "/";
        symLinkTarget.prepend(currPath);
    }

    return symLinkTarget;
}

QUrl LocalFileInfoPrivate::redirectedFileUrl() const
{
    if (q->isAttributes(AbstractFileInfo::FileIsType::kIsSymLink))
        return QUrl::fromLocalFile(symLinkTarget());
    return url;
}

/*!
 * \brief isExecutable 获取文件是否可执行
 *
 * \param
 *
 * \return bool 返回文件是否可执行
 */
bool LocalFileInfoPrivate::isExecutable() const
{
    bool isExecutable = false;
    bool success = false;
    if (dfmFileInfo) {
        QReadLocker locker(&const_cast<LocalFileInfoPrivate *>(this)->lock);
        isExecutable = dfmFileInfo->attribute(DFileInfo::AttributeID::kAccessCanExecute, &success).toBool();
    }
    if (!success) {
        qWarning() << "cannot obtain the property kAccessCanExecute of" << url;

        if (FileUtils::isGvfsFile(url)) {
            qInfo() << "trying to get isExecutable by judging whether the dir can be iterated" << url;
            struct dirent *next { nullptr };
            DIR *dirp = opendir(filePath().toUtf8().constData());
            if (!dirp) {
                isExecutable = false;
            } else {
                errno = 0;
                next = readdir(dirp);
                closedir(dirp);
                isExecutable = (next || errno == 0);
            }
            qInfo() << "dir can be iterated? " << isExecutable << url;
        }
    }

    return isExecutable;
}

bool LocalFileInfoPrivate::isPrivate() const
{
    const QString &path = const_cast<LocalFileInfoPrivate *>(this)->path();
    const QString &name = fileName();

    static DFMBASE_NAMESPACE::Match match("PrivateFiles");

    QReadLocker locker(&const_cast<LocalFileInfoPrivate *>(this)->lock);
    return match.match(path, name);
}

bool LocalFileInfoPrivate::canDelete() const
{
    if (SystemPathUtil::instance()->isSystemPath(filePath()))
        return false;

    bool canDelete = SysInfoUtils::isRootUser();
    if (!canDelete) {
        QReadLocker locker(&const_cast<LocalFileInfoPrivate *>(this)->lock);
        if (dfmFileInfo)
            canDelete = dfmFileInfo->attribute(DFileInfo::AttributeID::kAccessCanDelete, nullptr).toBool();
    }

    return canDelete;
}

bool LocalFileInfoPrivate::canTrash() const
{
    if (SystemPathUtil::instance()->isSystemPath(filePath()))
        return false;

    bool canTrash = false;
    if (!canTrash) {
        QReadLocker locker(&const_cast<LocalFileInfoPrivate *>(this)->lock);
        if (dfmFileInfo)
            canTrash = dfmFileInfo->attribute(DFileInfo::AttributeID::kAccessCanTrash, nullptr).toBool();
    }

    return canTrash;
}

bool LocalFileInfoPrivate::canRename() const
{
    if (SystemPathUtil::instance()->isSystemPath(filePath()))
        return false;

    bool canRename = false;
    canRename = SysInfoUtils::isRootUser();
    if (!canRename) {
        QReadLocker locker(&const_cast<LocalFileInfoPrivate *>(this)->lock);
        if (dfmFileInfo)
            canRename = dfmFileInfo->attribute(DFileInfo::AttributeID::kAccessCanRename, nullptr).toBool();
    }

    return canRename;
}

bool LocalFileInfoPrivate::canFetch() const
{
    if (isPrivate())
        return false;

    bool isArchive = false;
    if (q->exists())
        isArchive = DFMBASE_NAMESPACE::MimeTypeDisplayManager::
                            supportArchiveMimetypes()
                                    .contains(DMimeDatabase().mimeTypeForFile(url).name());

    return q->isAttributes(AbstractFileInfo::FileIsType::kIsDir)
            || (isArchive
                && Application::instance()->genericAttribute(Application::kPreviewCompressFile).toBool());
}
/*!
 * \brief sizeFormat 格式化大小
 * \return QString 大小格式化后的大小
 */
QString LocalFileInfoPrivate::sizeFormat() const
{
    if (q->isAttributes(AbstractFileInfo::FileIsType::kIsDir)) {
        return QStringLiteral("-");
    }

    qlonglong fileSize(q->size());
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
    return QString("%1%2").arg(sizeString(QString::number(fileSize, 'f', 1)), unitString);
}

}
