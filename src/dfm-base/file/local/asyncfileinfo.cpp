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
#include <dfm-base/utils/thumbnail/thumbnailfactory.h>

#include <dfm-io/dfmio_utils.h>
#include <dfm-io/dfileinfo.h>

#include <QDateTime>
#include <QDir>
#include <QMap>
#include <QApplication>
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

/*!
 * \brief exists 文件是否存在
 *
 * \param
 *
 * \return 返回文件是否存在
 */
bool AsyncFileInfo::exists() const
{
    return d->asyncAttribute(FileInfo::FileInfoAttributeID::kStandardFileExists).toBool();
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
    if (d->queringAttribute)
        return;

    {
        FileInfoHelper::instance().fileRefreshAsync(sharedFromThis());
        QWriteLocker locker(&d->lock);
        d->fileCountFuture.reset(nullptr);
        d->updateFileCountFuture.reset(nullptr);
        d->mediaFuture.reset(nullptr);
        d->extraProperties.clear();
        d->attributesExtend.clear();
        d->extendIDs.clear();
    }

    {
        QWriteLocker lk(&extendOtherCacheLock);
        extendOtherCache.remove(ExtInfoType::kFileThumbnail);
    }
}

void AsyncFileInfo::cacheAttribute(DFileInfo::AttributeID id, const QVariant &value)
{
    QWriteLocker locker(&d->lock);
    d->cacheAsyncAttributes.insert(static_cast<FileInfo::FileInfoAttributeID>(id), value);
}

QString AsyncFileInfo::nameOf(const NameInfoType type) const
{
    switch (type) {
    case FileNameInfoType::kFileName:
        if (d->asyncAttribute(FileInfo::FileInfoAttributeID::kStandardName).isValid())
            return d->asyncAttribute(FileInfo::FileInfoAttributeID::kStandardName).toString();
        break;
    case FileNameInfoType::kCompleteBaseName:
        if (d->asyncAttribute(FileInfo::FileInfoAttributeID::kStandardCompleteBaseName).isValid())
            return d->asyncAttribute(FileInfo::FileInfoAttributeID::kStandardCompleteBaseName).toString();
        break;
    case FileNameInfoType::kCompleteSuffix:
        if (d->asyncAttribute(FileInfo::FileInfoAttributeID::kStandardCompleteSuffix).isValid())
            return d->asyncAttribute(FileInfo::FileInfoAttributeID::kStandardCompleteSuffix).toString();
        break;
    case FileNameInfoType::kFileCopyName:
        if (d->asyncAttribute(FileInfo::FileInfoAttributeID::kStandardDisplayName).isValid())
            return d->asyncAttribute(FileInfo::FileInfoAttributeID::kStandardDisplayName).toString();
        break;
    case FileNameInfoType::kIconName:
        return d->iconName();
    case FileNameInfoType::kGenericIconName:
        return const_cast<AsyncFileInfo *>(this)->fileMimeType().genericIconName();
    case FileNameInfoType::kMimeTypeName:
        return d->mimeTypeName();
    default:
        return FileInfo::nameOf(type);
    }
    return FileInfo::nameOf(type);
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
        if (d->asyncAttribute(FileInfo::FileInfoAttributeID::kStandardFilePath).isValid())
            return d->asyncAttribute(FileInfo::FileInfoAttributeID::kStandardFilePath).toString();
        break;
    case FilePathInfoType::kPath:
        [[fallthrough]];
    case FilePathInfoType::kAbsolutePath:
        if (d->asyncAttribute(FileInfo::FileInfoAttributeID::kStandardParentPath).isValid())
            return d->asyncAttribute(FileInfo::FileInfoAttributeID::kStandardParentPath).toString();
        break;
    case FilePathInfoType::kSymLinkTarget:
        return d->asyncAttribute(FileInfo::FileInfoAttributeID::kStandardSymlinkTarget).toString();
    default:
        return FileInfo::pathOf(type);
    }
    return FileInfo::pathOf(type);
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
    case FileUrlInfoType::kOriginalUrl:{
        auto originalUri = d->asyncAttribute(FileInfo::FileInfoAttributeID::kOriginalUri);
        if (originalUri.isValid())
            return QUrl(originalUri.toString());
        auto tmp = d->dfmFileInfo;
        if (tmp) {
            return QUrl(tmp->attribute(DFileInfo::AttributeID::kOriginalUri).toString());
        } else {
            return FileInfo::urlOf(type);
        }
    }
    default:
        return FileInfo::urlOf(type);
    }
}

bool AsyncFileInfo::isAttributes(const OptInfoType type) const
{
    switch (type) {
    case FileIsType::kIsFile:
        return d->asyncAttribute(FileInfo::FileInfoAttributeID::kStandardIsFile).toBool();
    case FileIsType::kIsDir:
        return d->asyncAttribute(FileInfo::FileInfoAttributeID::kStandardIsDir).toBool();
    case FileIsType::kIsReadable:
        return d->asyncAttribute(FileInfo::FileInfoAttributeID::kAccessCanRead).toBool();
    case FileIsType::kIsWritable:
        return d->asyncAttribute(FileInfo::FileInfoAttributeID::kAccessCanWrite).toBool();
    case FileIsType::kIsHidden:
        return d->asyncAttribute(FileInfo::FileInfoAttributeID::kStandardIsHidden).toBool();
    case FileIsType::kIsSymLink:
        return d->asyncAttribute(FileInfo::FileInfoAttributeID::kStandardIsSymlink).toBool();
    case FileIsType::kIsExecutable:
        return d->asyncAttribute(FileInfo::FileInfoAttributeID::kAccessCanExecute).toBool();
    case FileIsType::kIsRoot:
        return d->asyncAttribute(FileInfo::FileInfoAttributeID::kStandardFilePath).toString() == "/";
    default:
        return FileInfo::isAttributes(type);
    }
}

bool AsyncFileInfo::canAttributes(const CanableInfoType type) const
{
    switch (type) {
    case FileCanType::kCanDelete:
        return d->asyncAttribute(FileInfo::FileInfoAttributeID::kAccessCanDelete).toBool();
    case FileCanType::kCanTrash:
        return d->asyncAttribute(FileInfo::FileInfoAttributeID::kAccessCanTrash).toBool();
    case FileCanType::kCanRename:
        return d->asyncAttribute(FileInfo::FileInfoAttributeID::kAccessCanRename).toBool();
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
        return false;
    case FileExtendedInfoType::kFileCdRomDevice:
        return d->asyncAttribute(FileInfo::FileInfoAttributeID::kStandardIsCdRomDevice).toBool();
    case FileExtendedInfoType::kSizeFormat:
        return d->sizeFormat();
    case FileExtendedInfoType::kInode:
        return d->asyncAttribute(FileInfo::FileInfoAttributeID::kUnixInode);
    case FileExtendedInfoType::kOwner:
        return d->asyncAttribute(FileInfo::FileInfoAttributeID::kOwnerUser);
    case FileExtendedInfoType::kGroup:
        return d->asyncAttribute(FileInfo::FileInfoAttributeID::kOwnerGroup);
    case FileExtendedInfoType::kFileIsHid:
        return d->asyncAttribute(FileInfo::FileInfoAttributeID::kStandardIsHidden);
    case FileExtendedInfoType::kOwnerId:
        return d->asyncAttribute(FileInfo::FileInfoAttributeID::kUnixUID);
    case FileExtendedInfoType::kGroupId:
        return d->asyncAttribute(FileInfo::FileInfoAttributeID::kUnixGID);
    default:
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

    ps = static_cast<QFileDevice::Permissions>(
            static_cast<uint16_t>(
                    d->asyncAttribute(FileInfo::FileInfoAttributeID::kAccessPermissions).value<DFile::Permissions>()));

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
    return d->asyncAttribute(FileInfo::FileInfoAttributeID::kStandardSize).value<qint64>();
}
/*!
 * \brief timeInfo 获取文件的时间信息
 *
 * \return QVariant 普通的返回QDateTime, second和msecond返回qint64
 */
QVariant AsyncFileInfo::timeOf(const TimeInfoType type) const
{
    switch (type) {
    case TimeInfoType::kCreateTime:
        return QDateTime::fromSecsSinceEpoch(d->asyncAttribute(FileInfo::FileInfoAttributeID::kTimeCreated).value<qint64>());
    case TimeInfoType::kBirthTime:
        return QDateTime::fromSecsSinceEpoch(d->asyncAttribute(FileInfo::FileInfoAttributeID::kTimeCreated).value<qint64>());
    case TimeInfoType::kMetadataChangeTime:
        return QDateTime::fromSecsSinceEpoch(d->asyncAttribute(FileInfo::FileInfoAttributeID::kTimeChanged).value<qint64>());
    case TimeInfoType::kLastModified:
        return QDateTime::fromSecsSinceEpoch(d->asyncAttribute(FileInfo::FileInfoAttributeID::kTimeModified).value<qint64>());
    case TimeInfoType::kLastRead:
        return QDateTime::fromSecsSinceEpoch(d->asyncAttribute(FileInfo::FileInfoAttributeID::kTimeAccess).value<qint64>());
    case TimeInfoType::kCreateTimeSecond:
        return d->asyncAttribute(FileInfo::FileInfoAttributeID::kTimeCreated).value<qint64>();
    case TimeInfoType::kBirthTimeSecond:
        return d->asyncAttribute(FileInfo::FileInfoAttributeID::kTimeCreated).value<qint64>();
    case TimeInfoType::kMetadataChangeTimeSecond:
        return d->asyncAttribute(FileInfo::FileInfoAttributeID::kTimeChanged).value<qint64>();
    case TimeInfoType::kLastModifiedSecond:
        return d->asyncAttribute(FileInfo::FileInfoAttributeID::kTimeModified).value<qint64>();
    case TimeInfoType::kLastReadSecond:
        return d->asyncAttribute(FileInfo::FileInfoAttributeID::kTimeAccess).value<qint64>();
    case TimeInfoType::kCreateTimeMSecond:
        return d->asyncAttribute(FileInfo::FileInfoAttributeID::kTimeCreatedUsec).value<qint64>();
    case TimeInfoType::kBirthTimeMSecond:
        return d->asyncAttribute(FileInfo::FileInfoAttributeID::kTimeCreatedUsec).value<qint64>();
    case TimeInfoType::kMetadataChangeTimeMSecond:
        return d->asyncAttribute(FileInfo::FileInfoAttributeID::kTimeChangedUsec).value<qint64>();
    case TimeInfoType::kLastModifiedMSecond:
        return d->asyncAttribute(FileInfo::FileInfoAttributeID::kTimeModifiedUsec).value<qint64>();
    case TimeInfoType::kLastReadMSecond:
        return d->asyncAttribute(FileInfo::FileInfoAttributeID::kTimeAccessUsec).value<qint64>();
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
    return d->asyncAttribute(FileInfo::FileInfoAttributeID::kStandardFileType).value<FileType>();
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
        QReadLocker rlocker(&d->lock);
        if (!d->fileCountFuture && !d->updateFileCountFuture) {
            rlocker.unlock();
            auto future = FileInfoHelper::instance().fileCountAsync(const_cast<AsyncFileInfo *>(this)->url);
            QWriteLocker wlocker(&d->lock);
            d->fileCountFuture = future;
        } else if (d->fileCountFuture && (!d->updateFileCountFuture || !d->updateFileCountFuture->finish)) {
            return d->fileCountFuture->finish ? d->fileCountFuture->data.toInt() : -1;
        } else if (!d->fileCountFuture && d->updateFileCountFuture) {
            rlocker.unlock();
            QWriteLocker wlocker(&d->lock);
            d->fileCountFuture = d->updateFileCountFuture;
            d->updateFileCountFuture.reset(nullptr);
            return d->fileCountFuture->finish ? d->fileCountFuture->data.toInt() : -1;
        }
    }
    return -1;
}

QString AsyncFileInfo::displayOf(const DisPlayInfoType type) const
{
    if (type == DisPlayInfoType::kFileDisplayName) {
        if (d->asyncAttribute(FileInfo::FileInfoAttributeID::kStandardDisplayName).isValid())
            return d->asyncAttribute(FileInfo::FileInfoAttributeID::kStandardDisplayName).toString();
        return url.fileName();
    }
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
    if (d->queringAttribute || d->cacheingAttributes) {
        QIcon icon;
        {   // if already loaded thumb just return it.
            QReadLocker rlk(&d->iconLock);
            icon = d->fileIcon;
        }
        if (!icon.isNull())
            return icon;

        return QIcon::fromTheme("unknown");
    }

    return d->defaultIcon();
}

QMimeType AsyncFileInfo::fileMimeType(QMimeDatabase::MatchMode mode)
{
    Q_UNUSED(mode);
    QReadLocker locker(&d->lock);
    return d->mimeType;
}

QMimeType AsyncFileInfo::fileMimeTypeAsync(QMimeDatabase::MatchMode mode)
{
    return fileMimeType(mode);
}

QString AsyncFileInfo::viewOfTip(const ViewType type) const
{
    if (type == ViewType::kEmptyDir) {
        if (!exists()) {
            if (d->hasAsyncAttribute(FileInfo::FileInfoAttributeID::kStandardFileExists))
                return QObject::tr("File has been moved or deleted");
            dfmio::DFile file(fileUrl());
            if (!file.exists())
                return QObject::tr("File has been moved or deleted");
        } else if (!isAttributes(FileIsType::kIsReadable)) {
            if (d->hasAsyncAttribute(FileInfo::FileInfoAttributeID::kAccessCanRead))
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
    if (d->queringAttribute || d->cacheingAttributes)
        return QVariant();
    auto tmpDfmFileInfo = d->dfmFileInfo;
    if (tmpDfmFileInfo && tmpDfmFileInfo->queryAttributeFinished())
        return tmpDfmFileInfo->customAttribute(key, type);

    return QVariant();
}

QVariant AsyncFileInfo::customData(int role) const
{
    using namespace dfmbase::Global;
    if (role == kItemFileRefreshIcon) {
        // kMimeTypeName
        {
            QWriteLocker lk(&extendOtherCacheLock);
            extendOtherCache.remove(ExtInfoType::kFileThumbnail);
        }
        QWriteLocker locker(&d->iconLock);
        if (d->fileIcon.name() == "unknown")
            d->fileIcon = QIcon();
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
        cacheAttribute(static_cast<DFileInfo::AttributeID>(FileInfo::FileInfoAttributeID::kStandardIsLocalDevice), value);
        break;
    case FileExtendedInfoType::kFileCdRomDevice:
        cacheAttribute(static_cast<DFileInfo::AttributeID>(FileInfo::FileInfoAttributeID::kStandardIsCdRomDevice), value);
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

void AsyncFileInfo::updateAttributes(const QList<FileInfo::FileInfoAttributeID> &types)
{
    auto typeAll = types;
    if (typeAll.isEmpty()) {
        if (isAttributes(OptInfoType::kIsSymLink)) {
            auto target = pathOf(PathInfoType::kSymLinkTarget);
            if (!target.isEmpty() && target != filePath()) {
                FileInfoPointer info = InfoFactory::create<FileInfo>(QUrl::fromLocalFile(target));
                if (info)
                    info->updateAttributes();
            }
        }
        // 更新所有属性
        typeAll.append(FileInfoAttributeID::kThumbnailIcon);
        typeAll.append(FileInfoAttributeID::kStandardIcon);

        typeAll.append(FileInfoAttributeID::kStandardSize);
        QReadLocker rlk(&d->lock);
        d->needUpdateMediaInfo = !d->attributesExtend.isEmpty();
    }
    // 更新缩略图
    if (typeAll.contains(FileInfoAttributeID::kThumbnailIcon)) {
        typeAll.removeOne(FileInfoAttributeID::kThumbnailIcon);
        d->updateThumbnail(url);
    }

    // 更新filecount
    if (typeAll.contains(FileInfoAttributeID::kFileCount)) {
        typeAll.removeOne(FileInfoAttributeID::kFileCount);
        QReadLocker rlocker(&d->lock);
        if (d->fileCountFuture
            && d->fileCountFuture->finish
            && (!d->updateFileCountFuture || d->updateFileCountFuture->finish)) {
            rlocker.unlock();
            auto future = FileInfoHelper::instance().fileCountAsync(const_cast<AsyncFileInfo *>(this)->url);
            QWriteLocker wlocker(&d->lock);
            d->updateFileCountFuture = future;
        }
    }

    // 更新mediaInfo
    if (typeAll.contains(FileInfoAttributeID::kFileMediaInfo)) {
        typeAll.removeOne(FileInfoAttributeID::kFileMediaInfo);
        // 再在缓存和正在查询都不调用，缓存时会去调用
        if (!d->cacheingAttributes && !d->queringAttribute) {
            DFileInfo::MediaType mediaType { DFileInfo::MediaType::kGeneral };
            QList<DFileInfo::AttributeExtendID> extendIDs;
            {
                QReadLocker lk(&d->lock);
                mediaType = d->mediaType;
                extendIDs = d->extendIDs;
            }
            if (!extendIDs.isEmpty())
                d->updateMediaInfo(mediaType, extendIDs);
        }
    }

    if (typeAll.isEmpty())
        return;

    // 其他属性都是从 dfm-io 的dfileinfo中获取，只需要异步刷新就行
    if (d->queringAttribute)
        return;
    d->init(fileUrl());
    FileInfoHelper::instance().fileRefreshAsync(sharedFromThis());
}

QMultiMap<QUrl, QString> AsyncFileInfo::notifyUrls() const
{
    QReadLocker lk(&const_cast<AsyncFileInfoPrivate *>(d.data())->notifyLock);
    return d->notifyUrls;
}
// if url is unvalid, it will clear all notify urls
void AsyncFileInfo::setNotifyUrl(const QUrl &url, const QString &infoPtr)
{
    assert(infoPtr != QString::number(quintptr(this), 16));
    if (!url.isValid()) {
        QWriteLocker lk(&d->notifyLock);
        d->notifyUrls.clear();
        return;
    }
    QWriteLocker lk(&d->notifyLock);
    if (!d->notifyUrls.contains(url, infoPtr)) {
        d->notifyUrls.insert(url, infoPtr);
    }
}

void AsyncFileInfo::removeNotifyUrl(const QUrl &url, const QString &infoPtr)
{
    QWriteLocker lk(&d->notifyLock);
    d->notifyUrls.remove(url, infoPtr);
}

int AsyncFileInfo::cacheAsyncAttributes(const QString &attributes)
{
    assert(qApp->thread() != QThread::currentThread());
    auto dfmFileInfo = d->dfmFileInfo;
    if (d->tokenKey != quintptr(dfmFileInfo.data()))
        return -1;

    if (d->cacheingAttributes)
        return 0;

    if (!d->cacheingAttributes)
        d->cacheingAttributes = true;
    auto result = d->cacheAllAttributes(attributes);
    d->cacheingAttributes = false;
    return result;
}

bool AsyncFileInfo::asyncQueryDfmFileInfo(int ioPriority, FileInfo::initQuerierAsyncCallback func, void *userData)
{
    if (d->queringAttribute)
        return false;
    d->queringAttribute = true;
    if (!d->notInit || !d->dfmFileInfo)
        d->init(url);

    d->notInit = false;
    if (!d->dfmFileInfo) {
        d->queringAttribute = false;
        return false;
    }
    d->dfmFileInfo->initQuerierAsync(ioPriority, func, userData);
    d->queringAttribute = false;
    return true;
}

int AsyncFileInfo::errorCodeFromDfmio() const
{
    auto dFileInfo = d->dfmFileInfo;
    if (dFileInfo)
        return dFileInfo->lastError();
    return -1;
}

void AsyncFileInfoPrivate::init(const QUrl &url, QSharedPointer<DFMIO::DFileInfo> dfileInfo)
{
    mimeTypeMode = QMimeDatabase::MatchDefault;
    if (url.isEmpty()) {
        qCWarning(logDFMBase, "Failed, can't use empty url init fileinfo");
        abort();
    }

    if (UrlRoute::isVirtual(url)) {
        qCWarning(logDFMBase, "Failed, can't use virtual scheme init local fileinfo");
        abort();
    }

    QUrl cvtResultUrl = QUrl::fromLocalFile(UrlRoute::urlToPath(url));

    if (!url.isValid()) {
        qCWarning(logDFMBase, "Failed, can't use valid url init fileinfo");
        abort();
    }

    if (dfileInfo) {
        notInit = true;
        dfmFileInfo = dfileInfo;
        tokenKey = quintptr(dfmFileInfo.data());
        return;
    }
    dfmFileInfo.reset(new DFileInfo(cvtResultUrl));
    if (!dfmFileInfo) {
        qCWarning(logDFMBase, "Failed, dfm-io use factory create fileinfo");
        abort();
    }
    tokenKey = quintptr(dfmFileInfo.data());
}

QMimeType AsyncFileInfoPrivate::mimeTypes(const QString &filePath, QMimeDatabase::MatchMode mode, const QString &inod, const bool isGvfs)
{
    DFMBASE_NAMESPACE::DMimeDatabase db;
    if (isGvfs) {
        return db.mimeTypeForFile(filePath, mode, inod, isGvfs);
    }
    return db.mimeTypeForFile(q->sharedFromThis(), mode);
}

QIcon AsyncFileInfoPrivate::defaultIcon()
{
    QIcon icon;
    {
        QReadLocker rlk(&iconLock);
        icon = fileIcon;
    }

    if (!icon.isNull())
        return icon;

    icon = updateIcon();

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
    if (SystemPathUtil::instance()->isSystemPath(asyncAttribute(FileInfo::FileInfoAttributeID::kStandardFilePath).toString()))
        iconNameValue = SystemPathUtil::instance()->systemPathIconNameByPath(asyncAttribute(FileInfo::FileInfoAttributeID::kStandardFilePath).toString());

    if (iconNameValue.isEmpty()) {
        const QStringList &list = asyncAttribute(FileInfo::FileInfoAttributeID::kStandardIcon).toStringList();
        if (!list.isEmpty())
            iconNameValue = list.first();
    }
    if (!FileUtils::isGvfsFile(q->fileUrl()) && iconNameValue.isEmpty())
        iconNameValue = q->fileMimeType().iconName();
    if (iconNameValue.isEmpty() && q->isAttributes(OptInfoType::kIsDir))
        iconNameValue = "folder";
    return iconNameValue;
}

QString AsyncFileInfoPrivate::mimeTypeName() const
{
    // At present, there is no dfmio library code. For temporary repair
    // local file use the method on v20 to obtain mimeType
    if (FileUtils::isGvfsFile(q->fileUrl())) {
        return asyncAttribute(FileInfo::FileInfoAttributeID::kStandardContentType).toString();
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
    bool ok = false;
    QString fileDisplayName = this->attribute(DFileInfo::AttributeID::kStandardDisplayName, &ok).toString();
    if (fileDisplayName.isEmpty() || !ok)
        fileDisplayName = q->fileUrl().fileName();
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
    return q->fileUrl().path();
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
    if (asyncAttribute(FileInfo::FileInfoAttributeID::kStandardIsSymlink).toBool())
        return QUrl::fromLocalFile(asyncAttribute(FileInfo::FileInfoAttributeID::kStandardSymlinkTarget).toString());
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
        qCDebug(logDFMBase) << "cannot obtain the property kAccessCanExecute of" << q->fileUrl();

        if (FileUtils::isGvfsFile(q->fileUrl())) {
            qCDebug(logDFMBase) << "trying to get isExecutable by judging whether the dir can be iterated" << q->fileUrl();
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
            qCDebug(logDFMBase) << "dir can be iterated? " << isExecutable << q->fileUrl();
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
    if (asyncAttribute(FileInfo::FileInfoAttributeID::kStandardIsDir).toBool()) {
        return QStringLiteral("-");
    }

    return FileUtils::formatSize(asyncAttribute(FileInfo::FileInfoAttributeID::kStandardSize).toLongLong());
}

QVariant AsyncFileInfoPrivate::attribute(DFileInfo::AttributeID key, bool *ok) const
{
    assert(qApp->thread() != QThread::currentThread());
    auto tmp = dfmFileInfo;
    if (tmp && tmp->queryAttributeFinished()) {
        bool getOk { false };
        auto value = tmp->attribute(key, &getOk);
        if (ok)
            *ok = getOk;
        return value;
    }
    return QVariant();
}

QVariant AsyncFileInfoPrivate::asyncAttribute(FileInfo::FileInfoAttributeID key) const
{
    QReadLocker lk(&const_cast<AsyncFileInfoPrivate *>(this)->lock);
    return cacheAsyncAttributes.value(key);
}

QMap<DFileInfo::AttributeExtendID, QVariant> AsyncFileInfoPrivate::mediaInfo(DFileInfo::MediaType type, QList<DFileInfo::AttributeExtendID> ids)
{
    auto tmpDfmFileInfo = dfmFileInfo;
    {
        QWriteLocker wlocker(&lock);
        mediaType = type;
        extendIDs = ids;
    }
    if (tmpDfmFileInfo && tmpDfmFileInfo->queryAttributeFinished() && !queringAttribute) {
        QReadLocker rlocker(&lock);
        auto it = ids.begin();
        while (it != ids.end()) {
            if (attributesExtend.count(*it))
                it = ids.erase(it);
            else
                ++it;
        }
        rlocker.unlock();
        if (!ids.isEmpty())
            updateMediaInfo(type, ids);
    } else {
        needUpdateMediaInfo = true;
    }

    QReadLocker rlocker(&lock);
    return attributesExtend;
}

FileInfo::FileType AsyncFileInfoPrivate::fileType() const
{
    FileInfo::FileType fileType { FileInfo::FileType::kUnknown };

    const QUrl &fileUrl = q->fileUrl();
    if (FileUtils::isTrashFile(fileUrl)
        && asyncAttribute(FileInfo::FileInfoAttributeID::kStandardIsSymlink).toBool()) {
        fileType = FileInfo::FileType::kRegularFile;
        return fileType;
    }

    // Cannot access statBuf.st_mode from the filesystem engine, so we have to stat again.
    // In addition we want to follow symlinks.
    const QString &absoluteFilePath = filePath();
    const QByteArray &nativeFilePath = QFile::encodeName(absoluteFilePath);
    QT_STATBUF statBuffer;
    auto fileMode = attribute(DFileInfo::AttributeID::kUnixMode).toUInt();
    if (fileMode <= 0 || QT_STAT(nativeFilePath.constData(), &statBuffer) != 0)
        return fileType;
    fileMode = fileMode <= 0 ? statBuffer.st_mode : fileMode;
    if (S_ISDIR(fileMode))
        fileType = FileInfo::FileType::kDirectory;
    else if (S_ISCHR(fileMode))
        fileType = FileInfo::FileType::kCharDevice;
    else if (S_ISBLK(fileMode))
        fileType = FileInfo::FileType::kBlockDevice;
    else if (S_ISFIFO(fileMode))
        fileType = FileInfo::FileType::kFIFOFile;
    else if (S_ISSOCK(fileMode))
        fileType = FileInfo::FileType::kSocketFile;
    else if (S_ISREG(fileMode))
        fileType = FileInfo::FileType::kRegularFile;

    return fileType;
}

int AsyncFileInfoPrivate::cacheAllAttributes(const QString &attributes)
{
    assert(qApp->thread() != QThread::currentThread());
    QMap<FileInfo::FileInfoAttributeID, QVariant> tmp;
    if (needUpdateMediaInfo) {
        DFileInfo::MediaType mediaType { DFileInfo::MediaType::kGeneral };
        QList<DFileInfo::AttributeExtendID> extendIDs;
        {
            QReadLocker lk(&lock);
            mediaType = this->mediaType;
            extendIDs = this->extendIDs;
        }
        updateMediaInfo(mediaType, extendIDs);
    }
    inserAsyncAttribute(FileInfo::FileInfoAttributeID::kStandardName, fileName());
    inserAsyncAttribute(FileInfo::FileInfoAttributeID::kStandardCompleteBaseName, completeBaseName());
    inserAsyncAttribute(FileInfo::FileInfoAttributeID::kStandardCompleteSuffix, completeSuffix());
    inserAsyncAttribute(FileInfo::FileInfoAttributeID::kStandardDisplayName, fileDisplayName());
    inserAsyncAttribute(FileInfo::FileInfoAttributeID::kOriginalUri, attribute(DFileInfo::AttributeID::kOriginalUri));
    if (q->size() > 0 && attribute(DFileInfo::AttributeID::kStandardSize).toLongLong() <= 0) {
        DFileInfo checkInfo(q->fileUrl());
        inserAsyncAttribute(FileInfo::FileInfoAttributeID::kStandardSize, checkInfo.attribute(DFileInfo::AttributeID::kStandardSize));
    } else {
        inserAsyncAttribute(FileInfo::FileInfoAttributeID::kStandardSize, attribute(DFileInfo::AttributeID::kStandardSize));
    }

    inserAsyncAttribute(FileInfo::FileInfoAttributeID::kStandardFilePath, filePath());
    inserAsyncAttribute(FileInfo::FileInfoAttributeID::kStandardParentPath, path());
    if (attributes.isEmpty() || attributes == "*") {
        inserAsyncAttribute(FileInfo::FileInfoAttributeID::kStandardFileExists, DFile(q->fileUrl()).exists());
    } else {
        inserAsyncAttribute(FileInfo::FileInfoAttributeID::kStandardFileExists, true);
    }
    // redirectedFileUrl
    auto symlink = symLinkTarget();
    if (attribute(DFileInfo::AttributeID::kStandardIsSymlink).toBool()
        && !symlink.isEmpty()
        && !FileUtils::isLocalDevice(QUrl::fromLocalFile(symlink))) {
        FileInfoPointer info = InfoFactory::create<FileInfo>(QUrl::fromLocalFile(symlink));
        auto asyncInfo = info.dynamicCast<AsyncFileInfo>();
        if (asyncInfo) {
            asyncInfo->setNotifyUrl(q->fileUrl(), QString::number(quintptr(q), 16));
            auto notifyUrls = q->notifyUrls();
            for (const auto &url : notifyUrls.keys()) {
                for (const auto &infoptr : notifyUrls.values(url))
                    asyncInfo->setNotifyUrl(url, infoptr);
            }
            asyncInfo->refresh();
        }
    }
    inserAsyncAttribute(FileInfo::FileInfoAttributeID::kStandardSymlinkTarget, symLinkTarget());
    inserAsyncAttribute(FileInfo::FileInfoAttributeID::kAccessCanRead, attribute(DFileInfo::AttributeID::kAccessCanRead));
    inserAsyncAttribute(FileInfo::FileInfoAttributeID::kAccessCanWrite, attribute(DFileInfo::AttributeID::kAccessCanWrite));
    inserAsyncAttribute(FileInfo::FileInfoAttributeID::kAccessCanExecute, isExecutable());
    inserAsyncAttribute(FileInfo::FileInfoAttributeID::kTimeAccess, attribute(DFileInfo::AttributeID::kTimeAccess));
    inserAsyncAttribute(FileInfo::FileInfoAttributeID::kTimeAccessUsec, attribute(DFileInfo::AttributeID::kTimeAccessUsec));
    if (!notInit)
        inserAsyncAttribute(FileInfo::FileInfoAttributeID::kStandardIsHidden, attribute(DFileInfo::AttributeID::kStandardIsHidden));
    inserAsyncAttribute(FileInfo::FileInfoAttributeID::kStandardIsFile, attribute(DFileInfo::AttributeID::kStandardIsFile));
    inserAsyncAttribute(FileInfo::FileInfoAttributeID::kStandardIsDir, attribute(DFileInfo::AttributeID::kStandardIsDir));
    inserAsyncAttribute(FileInfo::FileInfoAttributeID::kStandardIsSymlink, attribute(DFileInfo::AttributeID::kStandardIsSymlink));
    inserAsyncAttribute(FileInfo::FileInfoAttributeID::kAccessCanDelete, canDelete());
    inserAsyncAttribute(FileInfo::FileInfoAttributeID::kAccessCanTrash, canTrash());
    inserAsyncAttribute(FileInfo::FileInfoAttributeID::kAccessCanRename, canRename());
    if (!attributes.isEmpty() && attributes != "*")
        return 2;
    tmp.insert(FileInfo::FileInfoAttributeID::kOwnerUser, attribute(DFileInfo::AttributeID::kOwnerUser));
    tmp.insert(FileInfo::FileInfoAttributeID::kOwnerGroup, attribute(DFileInfo::AttributeID::kOwnerGroup));
    tmp.insert(FileInfo::FileInfoAttributeID::kUnixInode, attribute(DFileInfo::AttributeID::kUnixInode));
    tmp.insert(FileInfo::FileInfoAttributeID::kUnixUID, attribute(DFileInfo::AttributeID::kUnixUID));
    tmp.insert(FileInfo::FileInfoAttributeID::kUnixGID, attribute(DFileInfo::AttributeID::kUnixGID));
    tmp.insert(FileInfo::FileInfoAttributeID::kTimeCreated, attribute(DFileInfo::AttributeID::kTimeCreated));
    tmp.insert(FileInfo::FileInfoAttributeID::kTimeChanged, attribute(DFileInfo::AttributeID::kTimeChanged));
    tmp.insert(FileInfo::FileInfoAttributeID::kTimeModified, attribute(DFileInfo::AttributeID::kTimeModified));

    tmp.insert(FileInfo::FileInfoAttributeID::kTimeCreatedUsec, attribute(DFileInfo::AttributeID::kTimeCreatedUsec));
    tmp.insert(FileInfo::FileInfoAttributeID::kTimeChangedUsec, attribute(DFileInfo::AttributeID::kTimeChangedUsec));
    tmp.insert(FileInfo::FileInfoAttributeID::kTimeModifiedUsec, attribute(DFileInfo::AttributeID::kTimeModifiedUsec));

    tmp.insert(FileInfo::FileInfoAttributeID::kStandardFileType, QVariant::fromValue(fileType()));
    auto tmpdfmfileinfo = dfmFileInfo;
    if (tmpdfmfileinfo)
        tmp.insert(FileInfo::FileInfoAttributeID::kAccessPermissions, QVariant::fromValue(tmpdfmfileinfo->permissions()));
    // GenericIconName
    tmp.insert(FileInfo::FileInfoAttributeID::kStandardContentType, attribute(DFileInfo::AttributeID::kStandardContentType));
    // iconname
    tmp.insert(FileInfo::FileInfoAttributeID::kStandardIcon, attribute(DFileInfo::AttributeID::kStandardIcon));
    tmp.insert(FileInfo::FileInfoAttributeID::kStandardIsLocalDevice, false);
    tmp.insert(FileInfo::FileInfoAttributeID::kStandardIsCdRomDevice, FileUtils::isCdRomDevice(q->fileUrl()));
    if (q->nameOf(NameInfoType::kIconName) != attribute(DFileInfo::AttributeID::kStandardIcon)) {
        QWriteLocker rlk(&iconLock);
        fileIcon = QIcon();
    }

    {
        QWriteLocker lk(&changesLock);
        changesAttributes.clear();
        for (const auto &key : tmp.keys()) {
            if (inserAsyncAttribute(key, tmp.value(key)))
                changesAttributes.append(key);
        }

        if (changesAttributes.isEmpty())
            return 1;

        if (changesAttributes.contains(FileInfo::FileInfoAttributeID::kStandardFileType) || changesAttributes.contains(FileInfo::FileInfoAttributeID::kStandardFileExists) || changesAttributes.contains(FileInfo::FileInfoAttributeID::kStandardContentType))
            fileMimeTypeAsync();   // kMimeTypeName
    }

    return 2;
}

bool AsyncFileInfoPrivate::inserAsyncAttribute(const FileInfo::FileInfoAttributeID id, const QVariant &value)
{
    QWriteLocker lk(&lock);
    if (cacheAsyncAttributes.value(id) == value || !value.isValid())
        return false;
    cacheAsyncAttributes.insert(id, value);
    return true;
}

void AsyncFileInfoPrivate::fileMimeTypeAsync(QMimeDatabase::MatchMode mode)
{
    QMimeType type;
    type = mimeTypes(q->fileUrl().path(), mode);
    {
        QWriteLocker lk(&lock);
        mimeType = type;
        mimeTypeMode = mode;
    }
}

void AsyncFileInfoPrivate::updateThumbnail(const QUrl &url)
{
    ThumbnailFactory::instance()->joinThumbnailJob(url, Global::kLarge);
}

QIcon AsyncFileInfoPrivate::updateIcon()
{
    assert(QThread::currentThread() == qApp->thread());
    QIcon icon = LocalFileIconProvider::globalProvider()->icon(q->sharedFromThis());
    if (q->isAttributes(OptInfoType::kIsSymLink)) {
        const auto &&target = q->pathOf(PathInfoType::kSymLinkTarget);
        if (!target.isEmpty() && target != q->pathOf(PathInfoType::kFilePath)) {
            FileInfoPointer info = InfoFactory::create<FileInfo>(QUrl::fromLocalFile(target));
            if (info) {
                if (info->fileIcon().name() == "unknown")
                    info->customData(Global::ItemRoles::kItemFileRefreshIcon);
                icon = info->fileIcon();
            }
        }
    }

    {
        QWriteLocker wlk(&iconLock);
        fileIcon = icon;
    }
    return icon;
}

void AsyncFileInfoPrivate::updateMediaInfo(const DFileInfo::MediaType type, const QList<DFileInfo::AttributeExtendID> &ids)
{
    if (!dfmFileInfo || !dfmFileInfo->queryAttributeFinished() || queringAttribute)
        return;
    QReadLocker rlk(&lock);
    if (!ids.isEmpty() && !mediaFuture) {
        rlk.unlock();
        QWriteLocker wlk(&lock);
        mediaFuture.reset(new InfoDataFuture(dfmFileInfo->attributeExtend(type, ids, 0)));
    } else if (mediaFuture && mediaFuture->isFinished()) {
        rlk.unlock();
        QWriteLocker wlk(&lock);
        attributesExtend = mediaFuture->mediaInfo();
        mediaFuture.reset(nullptr);
    }
}

bool AsyncFileInfoPrivate::hasAsyncAttribute(FileInfo::FileInfoAttributeID key)
{
    QReadLocker lk(&lock);
    return cacheAsyncAttributes.contains(key);
}

}
