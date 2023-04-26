// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "asyncfileinfo.h"
#include "private/asyncfileinfo_p.h"

#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/standardpaths.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/chinese2pinyin.h>
#include <dfm-base/utils/sysinfoutils.h>
#include <dfm-base/file/local/localfileiconprovider.h>
#include <dfm-base/mimetype/dmimedatabase.h>
#include <dfm-base/mimetype/mimetypedisplaymanager.h>
#include <dfm-base/base/application/application.h>

#include <dfm-io/dfmio_utils.h>
#include <dfm-io/dfileinfo.h>

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

/*!
 * \class SyncFileInfo 本地文件信息类
 * \brief 内部实现本地文件的fileinfo，对应url的scheme是file://
 */
namespace dfmbase {

AsyncFileInfo::AsyncFileInfo(const QUrl &url)
    : FileInfo(url), d(new AsyncFileInfoPrivate(this))
{
    d->init(url);
}

AsyncFileInfo::AsyncFileInfo(const QUrl &url, QSharedPointer<DFileInfo> dfileInfo)
    : FileInfo(url), d(new AsyncFileInfoPrivate(this))
{
    d->init(url, dfileInfo);
}

AsyncFileInfo::~AsyncFileInfo()
{
    d = nullptr;
}

bool AsyncFileInfo::initQuerier()
{
    if (d->cacheing)
        return false;
    d->cacheing = true;
    if (!d->notInit || !d->dfmFileInfo)
        d->init(url);
    if (!d->dfmFileInfo) {
        d->cacheing = false;
        d->notInit = false;
        return false;
    }
    d->cacheAllAttributes();
    d->cacheing = false;
    d->notInit = false;
    return true;
}

/*!
 * \brief exists 文件是否存在
 *
 * \param
 *
 * \return 返回文件是否存在
 */
bool AsyncFileInfo::exists() const
{
    return d->asyncAttribute(AsyncAttributeID::kStandardFileExists).toBool();
}
/*!
 * \brief refresh 更新文件信息，清理掉缓存的所有的文件信息
 *
 * \param
 *
 * \return
 */
void AsyncFileInfo::refresh()
{
    {
        FileInfoHelper::instance().fileRefreshAsync(sharedFromThis());
        QWriteLocker locker(&d->lock);
        d->fileCountFuture.reset(nullptr);
        d->fileMimeTypeFuture.reset(nullptr);
        d->mediaFuture.reset(nullptr);
        d->extraProperties.clear();
        d->attributesExtend.clear();
        d->extendIDs.clear();
    }
}

void AsyncFileInfo::cacheAttribute(DFileInfo::AttributeID id, const QVariant &value)
{
    QWriteLocker locker(&d->lock);
    d->cacheAsyncAttributes.insert(static_cast<AsyncFileInfo::AsyncAttributeID>(id), value);
}

QString AsyncFileInfo::nameOf(const NameInfoType type) const
{
    switch (type) {
    case FileNameInfoType::kFileName:
        return d->asyncAttribute(AsyncAttributeID::kStandardName).toString();
    case FileNameInfoType::kBaseName:
        return d->asyncAttribute(AsyncAttributeID::kStandardBaseName).toString();
    case FileNameInfoType::kCompleteBaseName:
        return d->asyncAttribute(AsyncAttributeID::kStandardCompleteBaseName).toString();
    case FileNameInfoType::kSuffix:
        [[fallthrough]];
    case FileNameInfoType::kSuffixOfRename:
        return d->asyncAttribute(AsyncAttributeID::kStandardSuffix).toString();
    case FileNameInfoType::kCompleteSuffix:
        return d->asyncAttribute(AsyncAttributeID::kStandardCompleteSuffix).toString();
    case FileNameInfoType::kFileCopyName:
        return d->asyncAttribute(AsyncAttributeID::kStandardDisplayName).toString();
    case FileNameInfoType::kIconName:
        return d->iconName();
    case FileNameInfoType::kGenericIconName:
        return const_cast<AsyncFileInfo *>(this)->fileMimeType().genericIconName();
    case FileNameInfoType::kMimeTypeName:
        return d->mimeTypeName();
    default:
        return FileInfo::nameOf(type);
    }
}
/*!
  * \brief 获取文件路径，默认是文件全路径，此接口不会实现异步，全部使用Qurl去
  * 处理或者字符串处理，这都比较快
  * \param FileNameInfoType
  */
QString AsyncFileInfo::pathOf(const PathInfoType type) const
{
    switch (type) {
    case FilePathInfoType::kFilePath:
        [[fallthrough]];
    case FilePathInfoType::kAbsoluteFilePath:
        [[fallthrough]];
    case FilePathInfoType::kCanonicalPath:
        return d->asyncAttribute(AsyncAttributeID::kStandardFilePath).toString();
    case FilePathInfoType::kPath:
        [[fallthrough]];
    case FilePathInfoType::kAbsolutePath:
        return d->asyncAttribute(AsyncAttributeID::kStandardParentPath).toString();
    case FilePathInfoType::kSymLinkTarget:
        return d->asyncAttribute(AsyncAttributeID::kStandardSymlinkTarget).toString();
    default:
        return FileInfo::pathOf(type);
    }
}
/*!
 * \brief 获取文件url，默认是文件的url，此接口不会实现异步，全部使用Qurl去
 * 处理或者字符串处理，这都比较快
 * \param FileUrlInfoType
 */
QUrl AsyncFileInfo::urlOf(const UrlInfoType type) const
{
    switch (type) {
    case FileUrlInfoType::kRedirectedFileUrl:
        return d->redirectedFileUrl();
    default:
        return FileInfo::urlOf(type);
    }
}

bool AsyncFileInfo::isAttributes(const OptInfoType type) const
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
    case FileIsType::kIsExecutable:
        [[fallthrough]];
    case FileIsType::kIsSymLink:
        return d->asyncAttribute(d->getAttributeIDIsVector()[static_cast<int>(type)]).toBool();
    case FileIsType::kIsRoot:
        return d->asyncAttribute(AsyncAttributeID::kStandardFilePath).toString() == "/";
    default:
        return FileInfo::isAttributes(type);
    }
}

bool AsyncFileInfo::canAttributes(const CanableInfoType type) const
{
    switch (type) {
    case FileCanType::kCanDelete:
        return d->asyncAttribute(AsyncAttributeID::kAccessCanDelete).toBool();
    case FileCanType::kCanTrash:
        return d->asyncAttribute(AsyncAttributeID::kAccessCanTrash).toBool();
    case FileCanType::kCanRename:
        return d->asyncAttribute(AsyncAttributeID::kAccessCanRename).toBool();
    case FileCanType::kCanHidden:
        if (FileUtils::isGphotoFile(url))
            return false;
        return true;
    default:
        return FileInfo::canAttributes(type);
    }
}

QVariant AsyncFileInfo::extendAttributes(const ExtInfoType type) const
{
    switch (type) {
    case FileExtendedInfoType::kFileLocalDevice:
        return d->asyncAttribute(AsyncAttributeID::kStandardIsLocalDevice).toBool();
    case FileExtendedInfoType::kFileCdRomDevice:
        return d->asyncAttribute(AsyncAttributeID::kStandardIsCdRomDevice).toBool();
    case FileExtendedInfoType::kSizeFormat:
        return d->sizeFormat();
    case FileExtendedInfoType::kInode:
        [[fallthrough]];
    case FileExtendedInfoType::kOwner:
        [[fallthrough]];
    case FileExtendedInfoType::kGroup:
        [[fallthrough]];
    case FileExtendedInfoType::kFileIsHid:
        [[fallthrough]];
    case FileExtendedInfoType::kOwnerId:
        [[fallthrough]];
    case FileExtendedInfoType::kGroupId:
        return d->asyncAttribute(d->getAttributeIDExtendVector()[static_cast<int>(type)]);
    default:
        QReadLocker(&d->lock);
        return FileInfo::extendAttributes(type);
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
bool AsyncFileInfo::permission(QFileDevice::Permissions permissions) const
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
QFileDevice::Permissions AsyncFileInfo::permissions() const
{
    QFileDevice::Permissions ps;

    if (d->dfmFileInfo)
        ps = static_cast<QFileDevice::Permissions>(
                static_cast<uint16_t>(
                        d->asyncAttribute(AsyncAttributeID::kAccessPermissions).value<DFile::Permissions>()));

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
qint64 AsyncFileInfo::size() const
{
    return d->asyncAttribute(AsyncAttributeID::kStandardSize).value<qint64>();
}
/*!
 * \brief timeInfo 获取文件的时间信息
 *
 * \return QVariant 普通的返回QDateTime, second和msecond返回qint64
 */
QVariant AsyncFileInfo::timeOf(const TimeInfoType type) const
{
    qint64 data { 0 };
    if (type < FileTimeType::kDeletionTimeMSecond)
        data = d->asyncAttribute(d->getAttributeIDVector()[static_cast<int>(type)]).value<qint64>();

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
        return QDateTime::fromSecsSinceEpoch(data);
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
    case TimeInfoType::kCreateTimeMSecond:
        [[fallthrough]];
    case TimeInfoType::kBirthTimeMSecond:
        [[fallthrough]];
    case TimeInfoType::kMetadataChangeTimeMSecond:
        [[fallthrough]];
    case TimeInfoType::kLastModifiedMSecond:
        [[fallthrough]];
    case TimeInfoType::kLastReadMSecond:
        return data;
    default:
        return FileInfo::timeOf(type);
    }
}

/*!
 * \brief fileType 获取文件类型
 *
 * \return DMimeDatabase::FileType 文件设备类型
 */
AsyncFileInfo::FileType AsyncFileInfo::fileType() const
{
    return d->asyncAttribute(AsyncAttributeID::kStandardFileType).value<FileType>();
}
/*!
 * \brief countChildFile 文件夹下子文件的个数，只统计下一层不递归
 *
 * \return int 子文件个数
 */
int AsyncFileInfo::countChildFile() const
{
    return countChildFileAsync();
}

int AsyncFileInfo::countChildFileAsync() const
{
    if (isAttributes(FileIsType::kIsDir)) {
        QReadLocker locker(&d->lock);
        if (!d->fileCountFuture) {
            locker.unlock();
            auto future = FileInfoHelper::instance().fileCountAsync(const_cast<AsyncFileInfo *>(this)->url);
            QWriteLocker locker(&d->lock);
            d->fileCountFuture = future;
        } else {
            return d->fileCountFuture->finish ? d->fileCountFuture->data.toInt() : -1;
        }
    }
    return -1;
}

QString AsyncFileInfo::displayOf(const DisPlayInfoType type) const
{
    if (type == DisPlayInfoType::kFileDisplayName)
        return d->asyncAttribute(AsyncFileInfo::AsyncAttributeID::kStandardDisplayName).toString();
    return FileInfo::displayOf(type);
}

/*!
 * \brief extraProperties 获取文件的扩展属性
 *
 * \return QVariantHash 文件的扩展属性Hash
 */
QVariantHash AsyncFileInfo::extraProperties() const
{
    return d->extraProperties;
}

QIcon AsyncFileInfo::fileIcon()
{
    const QUrl &fileUrl = url;

    if (FileUtils::containsCopyingFileUrl(fileUrl))
        return LocalFileIconProvider::globalProvider()->icon(this);

    if (d->cacheing) {
        QIcon icon, deIcon;
        {   // if already loaded thumb just return it.
            QReadLocker rlk(&d->iconLock);
            icon = d->icons.value(AsyncFileInfoPrivate::IconType::kThumbIcon);
            deIcon = d->icons.value(AsyncFileInfoPrivate::IconType::kDefaultIcon);
        }
        if (!icon.isNull())
            return icon;

        if (!deIcon.isNull())
            return deIcon;

        return QIcon::fromTheme("unknown");
    }

#ifdef DFM_MINIMUM
    d->enableThumbnail = 0;
#else
    if (d->enableThumbnail < 0) {
        bool isLocalDevice = false;
        bool isCdRomDevice = false;

        if (d->asyncAttribute(AsyncFileInfo::AsyncAttributeID::kStandardIsLocalDevice).isValid())
            isLocalDevice = d->asyncAttribute(AsyncFileInfo::AsyncAttributeID::kStandardIsLocalDevice).toBool();
        else
            isLocalDevice = FileUtils::isLocalDevice(fileUrl);

        if (d->asyncAttribute(AsyncFileInfo::AsyncAttributeID::kStandardIsCdRomDevice).isValid())
            isCdRomDevice = d->asyncAttribute(AsyncFileInfo::AsyncAttributeID::kStandardIsCdRomDevice).toBool();
        else
            isCdRomDevice = FileUtils::isCdRomDevice(fileUrl);

        d->enableThumbnail = isLocalDevice && !isCdRomDevice;
    }
#endif
    bool thumbEnabled = (d->enableThumbnail > 0);
    if (thumbEnabled || ThumbnailProvider::instance()->thumbnailEnable(fileUrl))
        thumbEnabled = true;

    bool hasThumbnail = false;
    const int checkFast = ThumbnailProvider::instance()->hasThumbnailFast(d->mimeTypeName());
    if (1 == checkFast)
        hasThumbnail = true;
    else if (0 == checkFast)
        hasThumbnail = false;
    else
        hasThumbnail = ThumbnailProvider::instance()->hasThumbnail(fileMimeType());

    thumbEnabled = thumbEnabled && hasThumbnail;
    return thumbEnabled ? d->thumbIcon() : d->defaultIcon();
}

QMimeType AsyncFileInfo::fileMimeType(QMimeDatabase::MatchMode mode /*= QMimeDatabase::MatchDefault*/)
{
    Q_UNUSED(mode);
    QReadLocker locker(&d->lock);
    return d->mimeType;
}

QMimeType AsyncFileInfo::fileMimeTypeAsync(QMimeDatabase::MatchMode mode)
{
    QMimeType type;
    QMimeDatabase::MatchMode modeCache { QMimeDatabase::MatchMode::MatchDefault };

    QReadLocker rlk(&d->lock);
    type = d->mimeType;
    modeCache = d->mimeTypeMode;

    if (!d->fileMimeTypeFuture && (!type.isValid() || modeCache != mode)) {
        rlk.unlock();
        auto future = FileInfoHelper::instance().fileMimeTypeAsync(url, mode, QString(), false);
        QWriteLocker wlk(&d->lock);
        d->mimeType = type;
        d->mimeTypeMode = mode;
        d->fileMimeTypeFuture = future;
    } else if (d->fileMimeTypeFuture->finish) {
        type = d->fileMimeTypeFuture->data.value<QMimeType>();
        {
            QWriteLocker wlk(&d->iconLock);
            d->clearIcon();
        }
    }

    return type;
}

QString AsyncFileInfo::viewOfTip(const ViewType type) const
{
    if (type == ViewType::kEmptyDir) {
        if (!exists()) {
            return QObject::tr("File has been moved or deleted");
        } else if (!isAttributes(FileIsType::kIsReadable)) {
            return QObject::tr("You do not have permission to access this folder");
        } else if (isAttributes(FileIsType::kIsDir)) {
            if (!isAttributes(FileIsType::kIsExecutable))
                return QObject::tr("You do not have permission to traverse files in it");
        }
    }

    return FileInfo::viewOfTip(type);
}

QVariant AsyncFileInfo::customAttribute(const char *key, const DFileInfo::DFileAttributeType type)
{
    if (d->dfmFileInfo) {
        QReadLocker locker(&d->lock);
        return d->dfmFileInfo->customAttribute(key, type);
    }
    return QVariant();
}

QVariant AsyncFileInfo::customData(int role) const
{
    using namespace dfmbase::Global;
    if (role == kItemFileRefreshIcon) {
        QWriteLocker locker(&d->iconLock);
        d->clearIcon();
        return QVariant();
    }

    return FileInfo::customData(role);
}

QMap<DFMIO::DFileInfo::AttributeExtendID, QVariant> AsyncFileInfo::mediaInfoAttributes(DFileInfo::MediaType type, QList<DFileInfo::AttributeExtendID> ids) const
{
    return d->mediaInfo(type, ids);
}

void AsyncFileInfo::setExtendedAttributes(const FileExtendedInfoType &key, const QVariant &value)
{
    switch (key) {
    case FileExtendedInfoType::kFileLocalDevice:
        cacheAttribute(static_cast<DFileInfo::AttributeID>(AsyncFileInfo::AsyncAttributeID::kStandardIsLocalDevice), value);
        break;
    case FileExtendedInfoType::kFileCdRomDevice:
        cacheAttribute(static_cast<DFileInfo::AttributeID>(AsyncFileInfo::AsyncAttributeID::kStandardIsCdRomDevice), value);
        break;
    case FileExtendedInfoType::kFileIsHid: {
        cacheAttribute(DFileInfo::AttributeID::kStandardIsHidden, value);
        break;
    }
    default:
        FileInfo::setExtendedAttributes(key, value);
        break;
    }
}

QList<QUrl> AsyncFileInfo::notifyUrls() const
{
    QReadLocker lk(&const_cast<AsyncFileInfoPrivate *>(d.data())->notifyLock);
    return d->notifyUrls;
}
// if url is unvalid, it will clear all notify urls
void AsyncFileInfo::setNotifyUrl(const QUrl &url)
{
    if (!url.isValid()) {
        QWriteLocker lk(&d->notifyLock);
        d->notifyUrls.clear();
        return;
    }
    QWriteLocker lk(&d->notifyLock);
    if (!d->notifyUrls.contains(url))
        d->notifyUrls.append(url);
}

void AsyncFileInfoPrivate::init(const QUrl &url, QSharedPointer<DFMIO::DFileInfo> dfileInfo)
{
    mimeTypeMode = QMimeDatabase::MatchDefault;
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
        notInit = true;
        dfmFileInfo = dfileInfo;
        return;
    }
    dfmFileInfo.reset(new DFileInfo(cvtResultUrl));

    if (!dfmFileInfo) {
        qWarning("Failed, dfm-io use factory create fileinfo");
        abort();
    }
}

QMimeType AsyncFileInfoPrivate::mimeTypes(const QString &filePath, QMimeDatabase::MatchMode mode, const QString &inod, const bool isGvfs)
{
    static DFMBASE_NAMESPACE::DMimeDatabase db;
    if (isGvfs) {
        return db.mimeTypeForFile(filePath, mode, inod, isGvfs);
    }
    return db.mimeTypeForFile(q->sharedFromThis(), mode);
}

QIcon AsyncFileInfoPrivate::thumbIcon()
{
    QIcon icon;
    {   // if already loaded thumb just return it.
        QReadLocker rlk(&iconLock);
        icon = icons.value(IconType::kThumbIcon);
    }
    if (!icon.isNull())
        return icon;

    icon = QIcon(ThumbnailProvider::instance()->thumbnailPixmap(q->fileUrl(), ThumbnailProvider::kLarge));
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
    {
        QReadLocker rlk(&iconLock);
        if (canThumb()) {   // create thumbnail failed
            rlk.unlock();
            auto future = FileInfoHelper::instance().fileThumbAsync(q->fileUrl(), ThumbnailProvider::kLarge);
            QWriteLocker wlk(&iconLock);
            loadingThumbnail = true;
            iconFuture = future;
        }
    }

    return defaultIcon();
}

QIcon AsyncFileInfoPrivate::defaultIcon()
{
    QIcon icon;
    {
        QReadLocker rlk(&iconLock);
        icon = icons.value(AsyncFileInfoPrivate::kDefaultIcon);
    }

    if (!icon.isNull())
        return icon;

    icon = LocalFileIconProvider::globalProvider()->icon(q);
    if (q->isAttributes(OptInfoType::kIsSymLink)) {
        const auto &&target = symLinkTarget();
        if (target != filePath()) {
            FileInfoPointer info = InfoFactory::create<FileInfo>(QUrl::fromLocalFile(target));
            if (info)
                icon = info->fileIcon();
        }
    }

    {
        QWriteLocker wlk(&iconLock);
        icons.insert(AsyncFileInfoPrivate::kDefaultIcon, icon);
    }

    return icon;
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
QString AsyncFileInfoPrivate::fileName() const
{
    QString fileName = this->attribute(DFileInfo::AttributeID::kStandardName).toString();
    if (fileName == R"(/)" && FileUtils::isGvfsFile(q->fileUrl()))
        fileName = this->attribute(DFileInfo::AttributeID::kIdFilesystem).toString();
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
QString AsyncFileInfoPrivate::baseName() const
{
    return this->attribute(DFileInfo::AttributeID::kStandardBaseName).toString();
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
QString AsyncFileInfoPrivate::completeBaseName() const
{
    return this->attribute(DFileInfo::AttributeID::kStandardCompleteBaseName).toString();
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
QString AsyncFileInfoPrivate::suffix() const
{
    return this->attribute(DFileInfo::AttributeID::kStandardSuffix).toString();
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
QString AsyncFileInfoPrivate::completeSuffix() const
{
    return this->attribute(DFileInfo::AttributeID::kStandardCompleteSuffix).toString();
}

QString AsyncFileInfoPrivate::iconName() const
{
    QString iconNameValue;
    if (SystemPathUtil::instance()->isSystemPath(asyncAttribute(AsyncFileInfo::AsyncAttributeID::kStandardFilePath).toString()))
        iconNameValue = SystemPathUtil::instance()->systemPathIconNameByPath(asyncAttribute(AsyncFileInfo::AsyncAttributeID::kStandardFilePath).toString());

    if (iconNameValue.isEmpty()) {
        const QStringList &list = asyncAttribute(AsyncFileInfo::AsyncAttributeID::kStandardIcon).toStringList();
        if (!list.isEmpty())
            iconNameValue = list.first();
    }
    if (!FileUtils::isGvfsFile(q->fileUrl()) && iconNameValue.isEmpty())
        iconNameValue = q->fileMimeType().iconName();

    return iconNameValue;
}

QString AsyncFileInfoPrivate::mimeTypeName() const
{
    // At present, there is no dfmio library code. For temporary repair
    // local file use the method on v20 to obtain mimeType
    if (FileUtils::isGvfsFile(q->fileUrl())) {
        return asyncAttribute(AsyncFileInfo::AsyncAttributeID::kStandardContentType).toString();
    }
    return q->fileMimeType().name();
}

/*!
 * \brief fileDisplayName 文件的显示名称，一般为文件的名称
 *
 * \return QString 文件的显示名称
 */
QString AsyncFileInfoPrivate::fileDisplayName() const
{
    QString &&path { filePath() };
    if (SystemPathUtil::instance()->isSystemPath(path)) {
        QString displayName { SystemPathUtil::instance()->systemPathDisplayNameByPath(path) };
        if (!displayName.isEmpty())
            return displayName;
    }

    QString fileDisplayName = this->attribute(DFileInfo::AttributeID::kStandardDisplayName).toString();
    if (fileDisplayName == R"(/)" && FileUtils::isGvfsFile(q->fileUrl()))
        fileDisplayName = this->attribute(DFileInfo::AttributeID::kIdFilesystem).toString();

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
QString AsyncFileInfoPrivate::path() const
{
    return this->attribute(DFileInfo::AttributeID::kStandardParentPath).toString();
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
QString AsyncFileInfoPrivate::filePath() const
{
    return this->attribute(DFileInfo::AttributeID::kStandardFilePath).toString();
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
QString AsyncFileInfoPrivate::symLinkTarget() const
{
    QString symLinkTarget;

    if (dfmFileInfo) {
        symLinkTarget = this->attribute(DFileInfo::AttributeID::kStandardSymlinkTarget).toString();
    }
    // the link target may be a relative path.
    if (!symLinkTarget.startsWith(QDir::separator())) {
        auto currPath = path();
        if (currPath.right(1) != QDir::separator())
            currPath += QDir::separator();
        symLinkTarget.prepend(currPath);
    }

    return symLinkTarget;
}

QUrl AsyncFileInfoPrivate::redirectedFileUrl() const
{
    if (asyncAttribute(AsyncFileInfo::AsyncAttributeID::kStandardIsSymlink).toBool())
        return QUrl::fromLocalFile(asyncAttribute(AsyncFileInfo::AsyncAttributeID::kStandardSymlinkTarget).toString());
    return q->fileUrl();
}

/*!
 * \brief isExecutable 获取文件是否可执行
 *
 * \param
 *
 * \return bool 返回文件是否可执行
 */
bool AsyncFileInfoPrivate::isExecutable() const
{
    bool isExecutable = false;
    bool success = false;
    if (dfmFileInfo) {
        isExecutable = this->attribute(DFileInfo::AttributeID::kAccessCanExecute, &success).toBool();
    }
    if (!success) {
        qDebug() << "cannot obtain the property kAccessCanExecute of" << q->fileUrl();

        if (FileUtils::isGvfsFile(q->fileUrl())) {
            qDebug() << "trying to get isExecutable by judging whether the dir can be iterated" << q->fileUrl();
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
            qDebug() << "dir can be iterated? " << isExecutable << q->fileUrl();
        }
    }

    return isExecutable;
}

bool AsyncFileInfoPrivate::isPrivate() const
{
    const QString &path = const_cast<AsyncFileInfoPrivate *>(this)->path();
    const QString &name = fileName();

    static DFMBASE_NAMESPACE::Match match("PrivateFiles");

    QReadLocker locker(&const_cast<AsyncFileInfoPrivate *>(this)->lock);
    return match.match(path, name);
}

bool AsyncFileInfoPrivate::canDelete() const
{
    if (SystemPathUtil::instance()->isSystemPath(filePath()))
        return false;

    bool canDelete = SysInfoUtils::isRootUser();
    if (!canDelete)
        return this->attribute(DFileInfo::AttributeID::kAccessCanDelete).toBool();

    return canDelete;
}

bool AsyncFileInfoPrivate::canTrash() const
{
    if (SystemPathUtil::instance()->isSystemPath(filePath()))
        return false;

    bool canTrash = false;
    if (!canTrash)
        return this->attribute(DFileInfo::AttributeID::kAccessCanTrash).toBool();

    return canTrash;
}

bool AsyncFileInfoPrivate::canRename() const
{
    if (SystemPathUtil::instance()->isSystemPath(filePath()))
        return false;

    bool canRename = false;
    canRename = SysInfoUtils::isRootUser();
    if (!canRename)
        return this->attribute(DFileInfo::AttributeID::kAccessCanRename).toBool();

    return canRename;
}

bool AsyncFileInfoPrivate::canFetch() const
{
    if (isPrivate())
        return false;

    bool isArchive = false;
    if (q->exists())
        isArchive = DFMBASE_NAMESPACE::MimeTypeDisplayManager::instance()->supportArchiveMimetypes().contains(DMimeDatabase().mimeTypeForFile(q->fileUrl()).name());

    return q->isAttributes(OptInfoType::kIsDir)
            || (isArchive
                && Application::instance()->genericAttribute(Application::kPreviewCompressFile).toBool());
}
/*!
 * \brief sizeFormat 格式化大小
 * \return QString 大小格式化后的大小
 */
QString AsyncFileInfoPrivate::sizeFormat() const
{
    if (asyncAttribute(AsyncFileInfo::AsyncAttributeID::kStandardIsDir).toBool()) {
        return QStringLiteral("-");
    }

    return FileUtils::formatSize(cacheAsyncAttributes.value(AsyncFileInfo::AsyncAttributeID::kStandardSize).toInt());
}

QVariant AsyncFileInfoPrivate::attribute(DFileInfo::AttributeID key, bool *ok) const
{
    if (dfmFileInfo) {
        auto value = dfmFileInfo->attribute(key, ok);
        return value;
    }
    return QVariant();
}

QVariant AsyncFileInfoPrivate::asyncAttribute(AsyncFileInfo::AsyncAttributeID key) const
{
    QReadLocker lk(&const_cast<AsyncFileInfoPrivate *>(this)->lock);
    return cacheAsyncAttributes.value(key);
}

QMap<DFileInfo::AttributeExtendID, QVariant> AsyncFileInfoPrivate::mediaInfo(DFileInfo::MediaType type, QList<DFileInfo::AttributeExtendID> ids)
{
    if (dfmFileInfo) {
        extendIDs = ids;

        auto it = ids.begin();
        while (it != ids.end()) {
            if (attributesExtend.count(*it))
                it = ids.erase(it);
            else
                ++it;
        }

        if (!ids.isEmpty() && !mediaFuture) {
            mediaFuture.reset(new InfoDataFuture(dfmFileInfo->attributeExtend(type, ids, 0)));
        } else if (mediaFuture && mediaFuture->isFinished()) {
            attributesExtend = mediaFuture->mediaInfo();
            mediaFuture.reset(nullptr);
        }
    }

    return attributesExtend;
}

bool AsyncFileInfoPrivate::canThumb() const
{
    return !loadingThumbnail
            || (iconFuture && iconFuture->finish && iconFuture->data.toString().isEmpty());
}

FileInfo::FileType AsyncFileInfoPrivate::fileType() const
{
    FileInfo::FileType fileType { FileInfo::FileType::kUnknown };

    const QUrl &fileUrl = q->fileUrl();
    if (FileUtils::isTrashFile(fileUrl)
        && cacheAsyncAttributes.value(AsyncFileInfo::AsyncAttributeID::kStandardIsSymlink).toBool()) {
        fileType = FileInfo::FileType::kRegularFile;
        return fileType;
    }

    // Cannot access statBuf.st_mode from the filesystem engine, so we have to stat again.
    // In addition we want to follow symlinks.
    const QString &absoluteFilePath = filePath();
    const QByteArray &nativeFilePath = QFile::encodeName(absoluteFilePath);
    QT_STATBUF statBuffer;
    if (QT_STAT(nativeFilePath.constData(), &statBuffer) == 0) {
        if (S_ISDIR(statBuffer.st_mode))
            fileType = FileInfo::FileType::kDirectory;
        else if (S_ISCHR(statBuffer.st_mode))
            fileType = FileInfo::FileType::kCharDevice;
        else if (S_ISBLK(statBuffer.st_mode))
            fileType = FileInfo::FileType::kBlockDevice;
        else if (S_ISFIFO(statBuffer.st_mode))
            fileType = FileInfo::FileType::kFIFOFile;
        else if (S_ISSOCK(statBuffer.st_mode))
            fileType = FileInfo::FileType::kSocketFile;
        else if (S_ISREG(statBuffer.st_mode))
            fileType = FileInfo::FileType::kRegularFile;
    }
    return fileType;
}

void AsyncFileInfoPrivate::cacheAllAttributes()
{
    QMap<AsyncFileInfo::AsyncAttributeID, QVariant> tmp;
    tmp.insert(AsyncFileInfo::AsyncAttributeID::kStandardName, fileName());
    tmp.insert(AsyncFileInfo::AsyncAttributeID::kStandardBaseName, baseName());
    tmp.insert(AsyncFileInfo::AsyncAttributeID::kStandardCompleteBaseName, completeBaseName());
    tmp.insert(AsyncFileInfo::AsyncAttributeID::kStandardSuffix, suffix());
    tmp.insert(AsyncFileInfo::AsyncAttributeID::kStandardCompleteSuffix, completeSuffix());
    tmp.insert(AsyncFileInfo::AsyncAttributeID::kStandardDisplayName, fileDisplayName());
    tmp.insert(AsyncFileInfo::AsyncAttributeID::kStandardSize, attribute(DFileInfo::AttributeID::kStandardSize));
    tmp.insert(AsyncFileInfo::AsyncAttributeID::kStandardFilePath, filePath());
    tmp.insert(AsyncFileInfo::AsyncAttributeID::kStandardParentPath, path());
    tmp.insert(AsyncFileInfo::AsyncAttributeID::kStandardFileExists, dfmFileInfo->exists());
    // redirectedFileUrl
    auto symlink = symLinkTarget();
    if (attribute(DFileInfo::AttributeID::kStandardIsSymlink).toBool()
        && !symlink.isEmpty()
        && !FileUtils::isLocalDevice(QUrl::fromLocalFile(symlink))) {
        FileInfoPointer info = InfoFactory::create<FileInfo>(QUrl::fromLocalFile(symlink));
        auto asyncInfo = info.dynamicCast<AsyncFileInfo>();
        if (asyncInfo) {
            asyncInfo->setNotifyUrl(q->fileUrl());
            auto notifyUrls = q->notifyUrls();
            for (const auto &url : notifyUrls) {
                asyncInfo->setNotifyUrl(url);
            }
            asyncInfo->refresh();
        }
    }
    tmp.insert(AsyncFileInfo::AsyncAttributeID::kStandardSymlinkTarget, symLinkTarget());
    tmp.insert(AsyncFileInfo::AsyncAttributeID::kAccessCanRead, attribute(DFileInfo::AttributeID::kAccessCanRead));
    tmp.insert(AsyncFileInfo::AsyncAttributeID::kAccessCanWrite, attribute(DFileInfo::AttributeID::kAccessCanWrite));
    tmp.insert(AsyncFileInfo::AsyncAttributeID::kAccessCanExecute, isExecutable());
    if (!notInit)
        tmp.insert(AsyncFileInfo::AsyncAttributeID::kStandardIsHidden, attribute(DFileInfo::AttributeID::kStandardIsHidden));
    tmp.insert(AsyncFileInfo::AsyncAttributeID::kStandardIsFile, attribute(DFileInfo::AttributeID::kStandardIsFile));
    tmp.insert(AsyncFileInfo::AsyncAttributeID::kStandardIsDir, attribute(DFileInfo::AttributeID::kStandardIsDir));
    tmp.insert(AsyncFileInfo::AsyncAttributeID::kStandardIsSymlink, attribute(DFileInfo::AttributeID::kStandardIsSymlink));
    tmp.insert(AsyncFileInfo::AsyncAttributeID::kAccessCanDelete, canDelete());
    tmp.insert(AsyncFileInfo::AsyncAttributeID::kAccessCanTrash, canTrash());
    tmp.insert(AsyncFileInfo::AsyncAttributeID::kAccessCanRename, canRename());
    tmp.insert(AsyncFileInfo::AsyncAttributeID::kOwnerUser, attribute(DFileInfo::AttributeID::kOwnerUser));
    tmp.insert(AsyncFileInfo::AsyncAttributeID::kOwnerGroup, attribute(DFileInfo::AttributeID::kOwnerGroup));
    tmp.insert(AsyncFileInfo::AsyncAttributeID::kUnixInode, attribute(DFileInfo::AttributeID::kUnixInode));
    tmp.insert(AsyncFileInfo::AsyncAttributeID::kUnixUID, attribute(DFileInfo::AttributeID::kUnixUID));
    tmp.insert(AsyncFileInfo::AsyncAttributeID::kUnixGID, attribute(DFileInfo::AttributeID::kUnixGID));
    tmp.insert(AsyncFileInfo::AsyncAttributeID::kAccessPermissions, QVariant::fromValue(dfmFileInfo->permissions()));
    for (auto key : getAttributeIDVector()) {
        tmp.insert(key, attribute(static_cast<DFileInfo::AttributeID>(key)));
    }
    tmp.insert(AsyncFileInfo::AsyncAttributeID::kStandardFileType, QVariant::fromValue(fileType()));
    // GenericIconName
    tmp.insert(AsyncFileInfo::AsyncAttributeID::kStandardContentType, attribute(DFileInfo::AttributeID::kStandardContentType));
    // iconname
    tmp.insert(AsyncFileInfo::AsyncAttributeID::kStandardIcon, attribute(DFileInfo::AttributeID::kStandardIcon));
    tmp.insert(AsyncFileInfo::AsyncAttributeID::kStandardIsLocalDevice, FileUtils::isLocalDevice(q->fileUrl()));
    tmp.insert(AsyncFileInfo::AsyncAttributeID::kStandardIsCdRomDevice, FileUtils::isCdRomDevice(q->fileUrl()));
    {
        QWriteLocker lk(&lock);
        QVariant hid = cacheAsyncAttributes.value(AsyncFileInfo::AsyncAttributeID::kStandardIsHidden);
        cacheAsyncAttributes = tmp;
        if (notInit && hid.isValid())
            cacheAsyncAttributes.insert(AsyncFileInfo::AsyncAttributeID::kStandardIsHidden, hid);
    }

    // kMimeTypeName
    fileMimeTypeAsync();

    {
        QWriteLocker wlk(&iconLock);
        clearIcon();
    }
}

void AsyncFileInfoPrivate::fileMimeTypeAsync(QMimeDatabase::MatchMode mode)
{
    QMimeType type;
    bool clearIcons = false;
    type = mimeTypes(q->fileUrl().path(), mode);
    {
        QWriteLocker lk(&lock);
        clearIcons = mimeType != type;
        mimeType = type;
        mimeTypeMode = mode;
    }
    if (!icons.value(kThumbIcon).isNull() || clearIcons) {
        QWriteLocker lk(&iconLock);
        clearIcon();
    }
}

}
