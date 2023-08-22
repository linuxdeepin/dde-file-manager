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
    if (d->cacheing)
        return;

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
    {
        QWriteLocker locker(&extendOtherCacheLock);
        extendOtherCache.clear();
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
        if (d->asyncAttribute(AsyncAttributeID::kStandardName).isValid())
            return d->asyncAttribute(AsyncAttributeID::kStandardName).toString();
        break;
    case FileNameInfoType::kCompleteBaseName:
        if (d->asyncAttribute(AsyncAttributeID::kStandardCompleteBaseName).isValid())
            return d->asyncAttribute(AsyncAttributeID::kStandardCompleteBaseName).toString();
        break;
    case FileNameInfoType::kCompleteSuffix:
        if (d->asyncAttribute(AsyncAttributeID::kStandardCompleteSuffix).isValid())
            return d->asyncAttribute(AsyncAttributeID::kStandardCompleteSuffix).toString();
        break;
    case FileNameInfoType::kFileCopyName:
        if (d->asyncAttribute(AsyncAttributeID::kStandardDisplayName).isValid())
            return d->asyncAttribute(AsyncAttributeID::kStandardDisplayName).toString();
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
        if (d->asyncAttribute(AsyncAttributeID::kStandardFilePath).isValid())
            return d->asyncAttribute(AsyncAttributeID::kStandardFilePath).toString();
        break;
    case FilePathInfoType::kPath:
        [[fallthrough]];
    case FilePathInfoType::kAbsolutePath:
        if (d->asyncAttribute(AsyncAttributeID::kStandardParentPath).isValid())
            return d->asyncAttribute(AsyncAttributeID::kStandardParentPath).toString();
        break;
    case FilePathInfoType::kSymLinkTarget:
        return d->asyncAttribute(AsyncAttributeID::kStandardSymlinkTarget).toString();
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
    default:
        return FileInfo::urlOf(type);
    }
}

bool AsyncFileInfo::isAttributes(const OptInfoType type) const
{
    switch (type) {
    case FileIsType::kIsFile:
        return d->asyncAttribute(AsyncFileInfo::AsyncAttributeID::kStandardIsFile).toBool();
    case FileIsType::kIsDir:
        return d->asyncAttribute(AsyncFileInfo::AsyncAttributeID::kStandardIsDir).toBool();
    case FileIsType::kIsReadable:
        return d->asyncAttribute(AsyncFileInfo::AsyncAttributeID::kAccessCanRead).toBool();
    case FileIsType::kIsWritable:
        return d->asyncAttribute(AsyncFileInfo::AsyncAttributeID::kAccessCanWrite).toBool();
    case FileIsType::kIsHidden:
        return d->asyncAttribute(AsyncFileInfo::AsyncAttributeID::kStandardIsHidden).toBool();
    case FileIsType::kIsSymLink:
        return d->asyncAttribute(AsyncFileInfo::AsyncAttributeID::kStandardIsSymlink).toBool();
    case FileIsType::kIsExecutable:
        return d->asyncAttribute(AsyncFileInfo::AsyncAttributeID::kAccessCanExecute).toBool();
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
        return d->asyncAttribute(AsyncFileInfo::AsyncAttributeID::kUnixInode);
    case FileExtendedInfoType::kOwner:
        return d->asyncAttribute(AsyncFileInfo::AsyncAttributeID::kOwnerUser);
    case FileExtendedInfoType::kGroup:
        return d->asyncAttribute(AsyncFileInfo::AsyncAttributeID::kOwnerGroup);
    case FileExtendedInfoType::kFileIsHid:
        return d->asyncAttribute(AsyncFileInfo::AsyncAttributeID::kStandardIsHidden);
    case FileExtendedInfoType::kOwnerId:
        return d->asyncAttribute(AsyncFileInfo::AsyncAttributeID::kUnixUID);
    case FileExtendedInfoType::kGroupId:
        return d->asyncAttribute(AsyncFileInfo::AsyncAttributeID::kUnixGID);
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
    switch (type) {
    case TimeInfoType::kCreateTime:
        return QDateTime::fromSecsSinceEpoch(d->asyncAttribute(AsyncFileInfo::AsyncAttributeID::kTimeCreated).value<qint64>());
    case TimeInfoType::kBirthTime:
        return QDateTime::fromSecsSinceEpoch(d->asyncAttribute(AsyncFileInfo::AsyncAttributeID::kTimeCreated).value<qint64>());
    case TimeInfoType::kMetadataChangeTime:
        return QDateTime::fromSecsSinceEpoch(d->asyncAttribute(AsyncFileInfo::AsyncAttributeID::kTimeChanged).value<qint64>());
    case TimeInfoType::kLastModified:
        return QDateTime::fromSecsSinceEpoch(d->asyncAttribute(AsyncFileInfo::AsyncAttributeID::kTimeModified).value<qint64>());
    case TimeInfoType::kLastRead:
        return QDateTime::fromSecsSinceEpoch(d->asyncAttribute(AsyncFileInfo::AsyncAttributeID::kTimeAccess).value<qint64>());
    case TimeInfoType::kCreateTimeSecond:
        return d->asyncAttribute(AsyncFileInfo::AsyncAttributeID::kTimeCreated).value<qint64>();
    case TimeInfoType::kBirthTimeSecond:
        return d->asyncAttribute(AsyncFileInfo::AsyncAttributeID::kTimeCreated).value<qint64>();
    case TimeInfoType::kMetadataChangeTimeSecond:
        return d->asyncAttribute(AsyncFileInfo::AsyncAttributeID::kTimeChanged).value<qint64>();
    case TimeInfoType::kLastModifiedSecond:
        return d->asyncAttribute(AsyncFileInfo::AsyncAttributeID::kTimeModified).value<qint64>();
    case TimeInfoType::kLastReadSecond:
        return d->asyncAttribute(AsyncFileInfo::AsyncAttributeID::kTimeAccess).value<qint64>();
    case TimeInfoType::kCreateTimeMSecond:
        return d->asyncAttribute(AsyncFileInfo::AsyncAttributeID::kTimeCreatedUsec).value<qint64>();
    case TimeInfoType::kBirthTimeMSecond:
        return d->asyncAttribute(AsyncFileInfo::AsyncAttributeID::kTimeCreatedUsec).value<qint64>();
    case TimeInfoType::kMetadataChangeTimeMSecond:
        return d->asyncAttribute(AsyncFileInfo::AsyncAttributeID::kTimeChangedUsec).value<qint64>();
    case TimeInfoType::kLastModifiedMSecond:
        return d->asyncAttribute(AsyncFileInfo::AsyncAttributeID::kTimeModifiedUsec).value<qint64>();
    case TimeInfoType::kLastReadMSecond:
        return d->asyncAttribute(AsyncFileInfo::AsyncAttributeID::kTimeAccessUsec).value<qint64>();
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
    if (type == DisPlayInfoType::kFileDisplayName) {
        if (d->asyncAttribute(AsyncFileInfo::AsyncAttributeID::kStandardDisplayName).isValid())
            return d->asyncAttribute(AsyncFileInfo::AsyncAttributeID::kStandardDisplayName).toString();
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
    if (d->cacheing) {
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
    auto tmpDfmFileInfo = d->dfmFileInfo;
    if (tmpDfmFileInfo)
        return tmpDfmFileInfo->customAttribute(key, type);

    return QVariant();
}

QVariant AsyncFileInfo::customData(int role) const
{
    using namespace dfmbase::Global;
    if (role == kItemFileRefreshIcon) {
        {
            QWriteLocker lk(&extendOtherCacheLock);
            extendOtherCache.remove(ExtInfoType::kFileThumbnail);
        }
        QWriteLocker locker(&d->iconLock);
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

void AsyncFileInfo::cacheAsyncAttributes()
{
    assert(qApp->thread() != QThread::currentThread());
    if (!d->cacheing)
        d->cacheing = true;
    d->cacheAllAttributes();
    d->cacheing = false;
}

bool AsyncFileInfo::asyncQueryDfmFileInfo(int ioPriority, FileInfo::initQuerierAsyncCallback func, void *userData)
{
    if (d->cacheing)
        return false;
    d->cacheing = true;
    if (!d->notInit || !d->dfmFileInfo)
        d->init(url);

    d->notInit = false;
    if (!d->dfmFileInfo) {
        d->cacheing = false;
        return false;
    }

    d->dfmFileInfo->initQuerierAsync(ioPriority, func, userData);
    d->cacheing = false;
    return true;
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

    icon = LocalFileIconProvider::globalProvider()->icon(q);
    if (q->isAttributes(OptInfoType::kIsSymLink)) {
        const auto &&target = q->pathOf(PathInfoType::kSymLinkTarget);
        if (!target.isEmpty() && target != q->pathOf(PathInfoType::kFilePath)) {
            FileInfoPointer info = InfoFactory::create<FileInfo>(QUrl::fromLocalFile(target));
            if (info)
                icon = info->fileIcon();
        }
    }

    {
        QWriteLocker wlk(&iconLock);
        fileIcon = icon;
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

    return FileUtils::formatSize(cacheAsyncAttributes.value(AsyncFileInfo::AsyncAttributeID::kStandardSize).toLongLong());
}

QVariant AsyncFileInfoPrivate::attribute(DFileInfo::AttributeID key, bool *ok) const
{
    assert(qApp->thread() != QThread::currentThread());
    auto tmpDfmFileInfo = dfmFileInfo;
    if (tmpDfmFileInfo) {
        auto value = tmpDfmFileInfo->attribute(key, ok);
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
    auto tmpDfmFileInfo = dfmFileInfo;
    if (tmpDfmFileInfo) {
        extendIDs = ids;

        auto it = ids.begin();
        while (it != ids.end()) {
            if (attributesExtend.count(*it))
                it = ids.erase(it);
            else
                ++it;
        }

        if (!ids.isEmpty() && !mediaFuture) {
            mediaFuture.reset(new InfoDataFuture(tmpDfmFileInfo->attributeExtend(type, ids, 0)));
        } else if (mediaFuture && mediaFuture->isFinished()) {
            attributesExtend = mediaFuture->mediaInfo();
            mediaFuture.reset(nullptr);
        }
    }

    return attributesExtend;
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
    assert(qApp->thread() != QThread::currentThread());
    QMap<AsyncFileInfo::AsyncAttributeID, QVariant> tmp;
    tmp.insert(AsyncFileInfo::AsyncAttributeID::kStandardName, fileName());
    tmp.insert(AsyncFileInfo::AsyncAttributeID::kStandardCompleteBaseName, completeBaseName());
    tmp.insert(AsyncFileInfo::AsyncAttributeID::kStandardCompleteSuffix, completeSuffix());
    tmp.insert(AsyncFileInfo::AsyncAttributeID::kStandardDisplayName, fileDisplayName());
    tmp.insert(AsyncFileInfo::AsyncAttributeID::kStandardSize, attribute(DFileInfo::AttributeID::kStandardSize));
    tmp.insert(AsyncFileInfo::AsyncAttributeID::kStandardFilePath, filePath());
    tmp.insert(AsyncFileInfo::AsyncAttributeID::kStandardParentPath, path());
    auto tmpdfmfileinfo = dfmFileInfo;
    tmp.insert(AsyncFileInfo::AsyncAttributeID::kStandardFileExists, DFile(q->fileUrl()).exists());
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
    tmp.insert(AsyncFileInfo::AsyncAttributeID::kTimeCreated, attribute(DFileInfo::AttributeID::kTimeCreated));
    tmp.insert(AsyncFileInfo::AsyncAttributeID::kTimeChanged, attribute(DFileInfo::AttributeID::kTimeChanged));
    tmp.insert(AsyncFileInfo::AsyncAttributeID::kTimeModified, attribute(DFileInfo::AttributeID::kTimeModified));
    tmp.insert(AsyncFileInfo::AsyncAttributeID::kTimeAccess, attribute(DFileInfo::AttributeID::kTimeAccess));
    tmp.insert(AsyncFileInfo::AsyncAttributeID::kTimeCreatedUsec, attribute(DFileInfo::AttributeID::kTimeCreatedUsec));
    tmp.insert(AsyncFileInfo::AsyncAttributeID::kTimeChangedUsec, attribute(DFileInfo::AttributeID::kTimeChangedUsec));
    tmp.insert(AsyncFileInfo::AsyncAttributeID::kTimeModifiedUsec, attribute(DFileInfo::AttributeID::kTimeModifiedUsec));
    tmp.insert(AsyncFileInfo::AsyncAttributeID::kTimeAccessUsec, attribute(DFileInfo::AttributeID::kTimeAccessUsec));
    if (tmpdfmfileinfo)
        tmp.insert(AsyncFileInfo::AsyncAttributeID::kAccessPermissions, QVariant::fromValue(dfmFileInfo->permissions()));

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

}
