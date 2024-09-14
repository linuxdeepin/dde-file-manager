// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "syncfileinfo.h"
#include "private/syncfileinfo_p.h"

#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/standardpaths.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/chinese2pinyin.h>
#include <dfm-base/utils/sysinfoutils.h>
#include <dfm-base/file/local/localfileiconprovider.h>
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

SyncFileInfo::SyncFileInfo(const QUrl &url)
    : FileInfo(url), d(new SyncFileInfoPrivate(this))
{
    d->init(url);
}

SyncFileInfo::SyncFileInfo(const QUrl &url, QSharedPointer<DFileInfo> dfileInfo)
    : FileInfo(url), d(new SyncFileInfoPrivate(this))
{
    d->init(url, dfileInfo);
}

SyncFileInfo::~SyncFileInfo()
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
bool SyncFileInfo::operator==(const SyncFileInfo &fileinfo) const
{
    return d->dfmFileInfo == fileinfo.d->dfmFileInfo && url == fileinfo.url;
}
/*!
 * \brief != 重载操作符!=
 *
 * \param const SyncFileInfo & SyncFileInfo实例对象的引用
 *
 * \return bool 传入的SyncFileInfo实例对象和自己是否不相等
 */
bool SyncFileInfo::operator!=(const SyncFileInfo &fileinfo) const
{
    return !(operator==(fileinfo));
}

bool SyncFileInfo::initQuerier()
{
    if (d->dfmFileInfo)
        return d->dfmFileInfo->initQuerier();
    return false;
}

void SyncFileInfo::initQuerierAsync(int ioPriority, FileInfo::initQuerierAsyncCallback func, void *userData)
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
bool SyncFileInfo::exists() const
{
    return DFMIO::DFile(url.path()).exists();
}
/*!
 * \brief refresh 更新文件信息，清理掉缓存的所有的文件信息
 *
 * \param
 *
 * \return
 */
void SyncFileInfo::refresh()
{
    {
        QWriteLocker locker(&extendOtherCacheLock);
        extendOtherCache.clear();
    }
    QWriteLocker locker(&d->lock);
    d->dfmFileInfo->refresh();
    d->fileMimeTypeFuture.reset(nullptr);
    d->mediaFuture.reset(nullptr);
    d->fileType = FileInfo::FileType::kUnknown;
    d->mimeTypeMode = QMimeDatabase::MatchMode::MatchDefault;
    d->extraProperties.clear();
    d->attributesExtend.clear();
    d->extendIDs.clear();
    d->isLocalDevice = QVariant();
    d->isCdRomDevice = QVariant();
    d->mimeType = QMimeType();
    d->mimeTypeMode = QMimeDatabase::MatchDefault;
    d->cacheAttributes.clear();
    d->fileIcon = QIcon();
}

void SyncFileInfo::cacheAttribute(DFileInfo::AttributeID id, const QVariant &value)
{
    QWriteLocker locker(&d->lock);
    d->cacheAttributes.insert(id, value);
}

QString SyncFileInfo::nameOf(const NameInfoType type) const
{
    switch (type) {
    case FileNameInfoType::kFileName:
        return d->fileName();
    case FileNameInfoType::kCompleteBaseName:
        return d->completeBaseName();
    case FileNameInfoType::kCompleteSuffix:
        return d->completeSuffix();
    case FileNameInfoType::kFileCopyName:
        return d->fileDisplayName();
    case FileNameInfoType::kIconName:
        return d->iconName();
    case FileNameInfoType::kGenericIconName:
        return const_cast<SyncFileInfo *>(this)->fileMimeType().genericIconName();
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
QString SyncFileInfo::pathOf(const PathInfoType type) const
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
        return FileInfo::pathOf(type);
    }
}
/*!
 * \brief 获取文件url，默认是文件的url，此接口不会实现异步，全部使用Qurl去
 * 处理或者字符串处理，这都比较快
 * \param FileUrlInfoType
 */
QUrl SyncFileInfo::urlOf(const UrlInfoType type) const
{
    switch (type) {
    case FileUrlInfoType::kRedirectedFileUrl:
        return d->redirectedFileUrl();
    case FileUrlInfoType::kOriginalUrl:
        return d->attribute(DFileInfo::AttributeID::kOriginalUri).toUrl();
    default:
        return FileInfo::urlOf(type);
    }
}

bool SyncFileInfo::isAttributes(const OptInfoType type) const
{
    switch (type) {
    case FileIsType::kIsFile:
        return d->attribute(DFileInfo::AttributeID::kStandardIsFile).toBool();
    case FileIsType::kIsDir:
        return d->attribute(DFileInfo::AttributeID::kStandardIsDir).toBool();
    case FileIsType::kIsReadable:
        return d->attribute(DFileInfo::AttributeID::kAccessCanRead).toBool();
    case FileIsType::kIsWritable:
        return d->attribute(DFileInfo::AttributeID::kAccessCanWrite).toBool();
    case FileIsType::kIsHidden:
        return d->attribute(DFileInfo::AttributeID::kStandardIsHidden).toBool();
    case FileIsType::kIsSymLink:
        return d->attribute(DFileInfo::AttributeID::kStandardIsSymlink).toBool();
    case FileIsType::kIsExecutable:
        return d->isExecutable();
    case FileIsType::kIsRoot:
        return d->filePath() == "/";
    case FileIsType::kIsBundle:
        return QFileInfo(url.path()).isBundle();
    case FileIsType::kIsPrivate:
        return d->isPrivate();
    default:
        return FileInfo::isAttributes(type);
    }
}

bool SyncFileInfo::canAttributes(const CanableInfoType type) const
{
    switch (type) {
    case FileCanType::kCanRedirectionFileUrl:
        return d->attribute(DFileInfo::AttributeID::kStandardIsSymlink).toBool();
    case FileCanType::kCanDelete:
        return d->canDelete();
    case FileCanType::kCanTrash:
        return d->canTrash();
    case FileCanType::kCanRename:
        return d->canRename();
    case FileCanType::kCanHidden:
        if (FileUtils::isGphotoFile(url))
            return false;
        return true;
    default:
        return FileInfo::canAttributes(type);
    }
}

QVariant SyncFileInfo::extendAttributes(const ExtInfoType type) const
{
    switch (type) {
    case FileExtendedInfoType::kFileLocalDevice:
        return true;
    case FileExtendedInfoType::kFileCdRomDevice:
        return d->isCdRomDevice;
    case FileExtendedInfoType::kSizeFormat:
        return d->sizeFormat();
    case FileExtendedInfoType::kInode:
        return d->attribute(DFileInfo::AttributeID::kUnixInode);
    case FileExtendedInfoType::kOwner:
        return d->attribute(DFileInfo::AttributeID::kOwnerUser);
    case FileExtendedInfoType::kGroup:
        return d->attribute(DFileInfo::AttributeID::kOwnerGroup);
    case FileExtendedInfoType::kFileIsHid:
        return d->attribute(DFileInfo::AttributeID::kStandardIsHidden);
    case FileExtendedInfoType::kOwnerId:
        return d->attribute(DFileInfo::AttributeID::kUnixUID);
    case FileExtendedInfoType::kGroupId:
        return d->attribute(DFileInfo::AttributeID::kUnixGID);
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
bool SyncFileInfo::permission(QFileDevice::Permissions permissions) const
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
QFileDevice::Permissions SyncFileInfo::permissions() const
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
qint64 SyncFileInfo::size() const
{
    return d->attribute(DFileInfo::AttributeID::kStandardSize).value<qint64>();
}
/*!
 * \brief timeInfo 获取文件的时间信息
 *
 * \return QVariant 普通的返回QDateTime, second和msecond返回qint64
 */
QVariant SyncFileInfo::timeOf(const TimeInfoType type) const
{
    switch (type) {
    case TimeInfoType::kCreateTime:
        return QDateTime::fromSecsSinceEpoch(d->attribute(DFileInfo::AttributeID::kTimeCreated).value<qint64>());
    case TimeInfoType::kBirthTime:
        return QDateTime::fromSecsSinceEpoch(d->attribute(DFileInfo::AttributeID::kTimeCreated).value<qint64>());
    case TimeInfoType::kMetadataChangeTime:
        return QDateTime::fromSecsSinceEpoch(d->attribute(DFileInfo::AttributeID::kTimeChanged).value<qint64>());
    case TimeInfoType::kLastModified:
        return QDateTime::fromSecsSinceEpoch(d->attribute(DFileInfo::AttributeID::kTimeModified).value<qint64>());
    case TimeInfoType::kLastRead:
        return QDateTime::fromSecsSinceEpoch(d->attribute(DFileInfo::AttributeID::kTimeAccess).value<qint64>());
    case TimeInfoType::kCreateTimeSecond:
        return d->attribute(DFileInfo::AttributeID::kTimeCreated).value<qint64>();
    case TimeInfoType::kBirthTimeSecond:
        return d->attribute(DFileInfo::AttributeID::kTimeCreated).value<qint64>();
    case TimeInfoType::kMetadataChangeTimeSecond:
        return d->attribute(DFileInfo::AttributeID::kTimeChanged).value<qint64>();
    case TimeInfoType::kLastModifiedSecond:
        return d->attribute(DFileInfo::AttributeID::kTimeModified).value<qint64>();
    case TimeInfoType::kLastReadSecond:
        return d->attribute(DFileInfo::AttributeID::kTimeAccess).value<qint64>();
    case TimeInfoType::kCreateTimeMSecond:
        return d->attribute(DFileInfo::AttributeID::kTimeCreatedUsec).value<qint64>();
    case TimeInfoType::kBirthTimeMSecond:
        return d->attribute(DFileInfo::AttributeID::kTimeCreatedUsec).value<qint64>();
    case TimeInfoType::kMetadataChangeTimeMSecond:
        return d->attribute(DFileInfo::AttributeID::kTimeChangedUsec).value<qint64>();
    case TimeInfoType::kLastModifiedMSecond:
        return d->attribute(DFileInfo::AttributeID::kTimeModifiedUsec).value<qint64>();
    case TimeInfoType::kLastReadMSecond:
        return d->attribute(DFileInfo::AttributeID::kTimeAccessUsec).value<qint64>();
    default:
        return FileInfo::timeOf(type);
    }
}

/*!
 * \brief fileType 获取文件类型
 *
 * \return DMimeDatabase::FileType 文件设备类型
 */
SyncFileInfo::FileType SyncFileInfo::fileType() const
{
    FileType fileType { FileType::kUnknown };
    {
        QReadLocker locker(&d->lock);
        if (d->fileType != FileInfo::FileType::kUnknown) {
            fileType = FileType(d->fileType);
            return fileType;
        }
    }
    return d->updateFileType();
}
/*!
 * \brief countChildFile 文件夹下子文件的个数，只统计下一层不递归
 *
 * \return int 子文件个数
 */
int SyncFileInfo::countChildFile() const
{
    return isAttributes(FileIsType::kIsDir) ? FileUtils::dirFfileCount(url) : -1;
}

int SyncFileInfo::countChildFileAsync() const
{
    return countChildFile();
}

QString SyncFileInfo::displayOf(const DisPlayInfoType type) const
{
    if (type == DisPlayInfoType::kFileDisplayName)
        return d->fileDisplayName();
    return FileInfo::displayOf(type);
}

/*!
 * \brief extraProperties 获取文件的扩展属性
 *
 * \return QVariantHash 文件的扩展属性Hash
 */
QVariantHash SyncFileInfo::extraProperties() const
{
    return d->extraProperties;
}

QIcon SyncFileInfo::fileIcon()
{
    QIcon icon;
    {
        QReadLocker rlk(&d->iconLock);
        icon = d->fileIcon;
    }

    if (!icon.isNull())
        return icon;

    return d->updateIcon();
}

QMimeType SyncFileInfo::fileMimeType(QMimeDatabase::MatchMode mode /*= QMimeDatabase::MatchDefault*/)
{
    QMimeType type;
    QMimeDatabase::MatchMode modeCache { QMimeDatabase::MatchMode::MatchDefault };
    {
        QReadLocker locker(&d->lock);
        type = d->mimeType;
        modeCache = d->mimeTypeMode;
    }

    if (!type.isValid() || modeCache != mode) {
        type = d->mimeTypes(url.path(), mode);
        QWriteLocker locker(&d->lock);
        d->mimeType = type;
        d->mimeTypeMode = mode;
    }

    return type;
}

QMimeType SyncFileInfo::fileMimeTypeAsync(QMimeDatabase::MatchMode mode)
{
    QMimeType type;
    QMimeDatabase::MatchMode modeCache { QMimeDatabase::MatchMode::MatchDefault };

    QReadLocker rlk(&d->lock);
    type = d->mimeType;
    modeCache = d->mimeTypeMode;

    if (d->fileMimeTypeFuture.isNull() && (!type.isValid() || modeCache != mode)) {
        rlk.unlock();
        auto future = FileInfoHelper::instance().fileMimeTypeAsync(url, mode, QString(), false);
        QWriteLocker wlk(&d->lock);
        d->mimeType = type;
        d->mimeTypeMode = mode;
        d->fileMimeTypeFuture = future;
    } else if (!d->fileMimeTypeFuture.isNull() && d->fileMimeTypeFuture->finish) {
        type = d->fileMimeTypeFuture->data.value<QMimeType>();
        rlk.unlock();
        QWriteLocker wlk(&d->lock);
        d->mimeType = type;
        d->mimeTypeMode = mode;
    }

    return type;
}

QString SyncFileInfo::viewOfTip(const ViewType type) const
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

    return FileInfo::viewOfTip(type);
}

QVariant SyncFileInfo::customAttribute(const char *key, const DFileInfo::DFileAttributeType type)
{
    if (d->dfmFileInfo) {
        QReadLocker locker(&d->lock);
        return d->dfmFileInfo->customAttribute(key, type);
    }
    return QVariant();
}

QMap<DFMIO::DFileInfo::AttributeExtendID, QVariant> SyncFileInfo::mediaInfoAttributes(DFileInfo::MediaType type, QList<DFileInfo::AttributeExtendID> ids) const
{
    return d->mediaInfo(type, ids);
}

void SyncFileInfo::setExtendedAttributes(const FileExtendedInfoType &key, const QVariant &value)
{
    QWriteLocker locker(&d->lock);
    switch (key) {
    case FileExtendedInfoType::kFileLocalDevice:
        d->isLocalDevice = value;
        break;
    case FileExtendedInfoType::kFileCdRomDevice:
        d->isCdRomDevice = value;
        break;
    case FileExtendedInfoType::kFileIsHid: {
        locker.unlock();
        cacheAttribute(DFileInfo::AttributeID::kStandardIsHidden, value);
        break;
    }
    default:
        FileInfo::setExtendedAttributes(key, value);
        break;
    }
}

void SyncFileInfo::updateAttributes(const QList<FileInfo::FileInfoAttributeID> &types)
{
    auto typeAll = types;
    if (typeAll.isEmpty()) {
        if (isAttributes(OptInfoType::kIsSymLink)) {
            auto target = d->symLinkTarget();
            if (!target.isEmpty() && target != filePath()) {
                FileInfoPointer info = InfoFactory::create<FileInfo>(QUrl::fromLocalFile(target));
                if (info)
                    info->updateAttributes();
            }
        }
        // 更新所有属性
        typeAll.append(FileInfoAttributeID::kThumbnailIcon);
        typeAll.append(FileInfoAttributeID::kStandardFileType);
        typeAll.append(FileInfoAttributeID::kStandardIcon);
        typeAll.append(FileInfoAttributeID::kFileMediaInfo);
        typeAll.append(FileInfoAttributeID::kFileMimeType);

        typeAll.append(FileInfoAttributeID::kStandardSize);
    }
    // 更新缩略图
    if (typeAll.contains(FileInfoAttributeID::kThumbnailIcon)) {
        typeAll.removeOne(FileInfoAttributeID::kThumbnailIcon);
        ThumbnailFactory::instance()->joinThumbnailJob(url, Global::kLarge);
    }

    // 更新fileicon
    if (typeAll.contains(FileInfoAttributeID::kStandardIcon)) {
        QWriteLocker wlk(&d->iconLock);
        d->fileIcon = QIcon();
    }

    // 更新mediaInfo
    if (typeAll.contains(FileInfoAttributeID::kFileMediaInfo)) {
        typeAll.removeOne(FileInfoAttributeID::kFileMediaInfo);
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

    // 更新filemimetype
    if (typeAll.contains(FileInfoAttributeID::kFileMimeType)) {
        typeAll.removeOne(FileInfoAttributeID::kFileMimeType);
        QMimeType type;
        QMimeDatabase::MatchMode modeCache { QMimeDatabase::MatchMode::MatchDefault };
        {
            QReadLocker locker(&d->lock);
            modeCache = d->mimeTypeMode;
        }
        type = d->mimeTypes(url.path(), modeCache);
        QWriteLocker locker(&d->lock);
        d->mimeType = type;
    }

    if (typeAll.isEmpty())
        return;

    QWriteLocker locker(&d->lock);
    d->init(fileUrl());
}

void SyncFileInfoPrivate::init(const QUrl &url, QSharedPointer<DFMIO::DFileInfo> dfileInfo)
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
        dfmFileInfo = dfileInfo;
        return;
    }

    dfmFileInfo.reset(new DFileInfo(cvtResultUrl));

    if (!dfmFileInfo) {
        qCWarning(logDFMBase, "Failed, dfm-io use factory create fileinfo");
        abort();
    }
}

QMimeType SyncFileInfoPrivate::mimeTypes(const QString &filePath, QMimeDatabase::MatchMode mode, const QString &inod, const bool isGvfs)
{
    DFMBASE_NAMESPACE::DMimeDatabase db;
    if (isGvfs) {
        return db.mimeTypeForFile(filePath, mode, inod, isGvfs);
    }
    return db.mimeTypeForFile(q->sharedFromThis(), mode);
}

FileInfo::FileType SyncFileInfoPrivate::updateFileType()
{
    FileInfo::FileType fileType = FileInfo::FileType::kUnknown;
    const QUrl &fileUrl = q->fileUrl();
    if (FileUtils::isTrashFile(fileUrl) && q->isAttributes(FileInfo::FileIsType::kIsSymLink)) {
        {
            QWriteLocker locker(&lock);
            this->fileType = FileInfo::FileType::kRegularFile;
        }
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

    QWriteLocker locker(&lock);
    this->fileType = FileInfo::FileType(fileType);

    return fileType;
}

QIcon SyncFileInfoPrivate::updateIcon()
{
    assert(QThread::currentThread() == qApp->thread());
    QIcon icon = LocalFileIconProvider::globalProvider()->icon(q->sharedFromThis());
    if (q->isAttributes(OptInfoType::kIsSymLink)) {
        const auto &&target = symLinkTarget();
        if (!target.isEmpty() && target != filePath()) {
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

void SyncFileInfoPrivate::updateMediaInfo(const DFileInfo::MediaType type, const QList<DFileInfo::AttributeExtendID> &ids)
{
    QReadLocker rlocker(&lock);
    if (!ids.isEmpty() && !mediaFuture) {
        rlocker.unlock();
        QWriteLocker wlocker(&lock);
        mediaFuture.reset(new InfoDataFuture(dfmFileInfo->attributeExtend(type, ids, 0)));
    } else if (mediaFuture && mediaFuture->isFinished()) {
        rlocker.unlock();
        QWriteLocker wlocker(&lock);
        attributesExtend = mediaFuture->mediaInfo();
        mediaFuture.reset(nullptr);
    }
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
QString SyncFileInfoPrivate::fileName() const
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
QString SyncFileInfoPrivate::completeBaseName() const
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
QString SyncFileInfoPrivate::completeSuffix() const
{
    return this->attribute(DFileInfo::AttributeID::kStandardCompleteSuffix).toString();
}

QString SyncFileInfoPrivate::iconName() const
{
    assert(QThread::currentThread() == qApp->thread());
    QString iconNameValue;
    if (SystemPathUtil::instance()->isSystemPath(filePath()))
        iconNameValue = SystemPathUtil::instance()->systemPathIconNameByPath(filePath());

    if (iconNameValue.isEmpty()) {
        const QStringList &list = this->attribute(DFileInfo::AttributeID::kStandardIcon).toStringList();
        const auto &iter = std::find_if(list.begin(), list.end(), [](const QString &name) { return QIcon::hasThemeIcon(name); });
        if (iter != list.end())
            iconNameValue = *iter;
    }

    if (!FileUtils::isGvfsFile(q->fileUrl()) && iconNameValue.isEmpty())
        iconNameValue = q->fileMimeType().iconName();

    return iconNameValue;
}

QString SyncFileInfoPrivate::mimeTypeName() const
{
    // At present, there is no dfmio library code. For temporary repair
    // local file use the method on v20 to obtain mimeType
    if (FileUtils::isGvfsFile(q->fileUrl())) {
        return this->attribute(DFileInfo::AttributeID::kStandardContentType).toString();
    }
    return q->fileMimeType().name();
}

/*!
 * \brief fileDisplayName 文件的显示名称，一般为文件的名称
 *
 * \return QString 文件的显示名称
 */
QString SyncFileInfoPrivate::fileDisplayName() const
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
QString SyncFileInfoPrivate::path() const
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
QString SyncFileInfoPrivate::filePath() const
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
QString SyncFileInfoPrivate::symLinkTarget() const
{
    QString symLinkTarget;

    if (dfmFileInfo) {
        symLinkTarget = this->attribute(DFileInfo::AttributeID::kStandardSymlinkTarget).toString();
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

QUrl SyncFileInfoPrivate::redirectedFileUrl() const
{
    if (q->isAttributes(OptInfoType::kIsSymLink))
        return QUrl::fromLocalFile(symLinkTarget());
    return q->fileUrl();
}

/*!
 * \brief isExecutable 获取文件是否可执行
 *
 * \param
 *
 * \return bool 返回文件是否可执行
 */
bool SyncFileInfoPrivate::isExecutable() const
{
    bool isExecutable = false;
    bool success = false;
    if (dfmFileInfo) {
        isExecutable = this->attribute(DFileInfo::AttributeID::kAccessCanExecute, &success).toBool();
    }
    if (!success) {
        qCWarning(logDFMBase) << "cannot obtain the property kAccessCanExecute of" << q->fileUrl();

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

bool SyncFileInfoPrivate::isPrivate() const
{
    const QString &path = const_cast<SyncFileInfoPrivate *>(this)->path();
    const QString &name = fileName();

    static DFMBASE_NAMESPACE::Match match("PrivateFiles");

    QReadLocker locker(&const_cast<SyncFileInfoPrivate *>(this)->lock);
    return match.match(path, name);
}

bool SyncFileInfoPrivate::canDelete() const
{
    if (SystemPathUtil::instance()->isSystemPath(filePath()))
        return false;

    bool canDelete = SysInfoUtils::isRootUser();
    if (!canDelete)
        return this->attribute(DFileInfo::AttributeID::kAccessCanDelete).toBool();

    return canDelete;
}

bool SyncFileInfoPrivate::canTrash() const
{
    if (SystemPathUtil::instance()->isSystemPath(filePath()))
        return false;

    bool canTrash = false;
    if (!canTrash)
        return this->attribute(DFileInfo::AttributeID::kAccessCanTrash).toBool();

    return canTrash;
}

bool SyncFileInfoPrivate::canRename() const
{
    if (SystemPathUtil::instance()->isSystemPath(filePath()))
        return false;

    bool canRename = false;
    canRename = SysInfoUtils::isRootUser();
    if (!canRename)
        return this->attribute(DFileInfo::AttributeID::kAccessCanRename).toBool();

    return canRename;
}

bool SyncFileInfoPrivate::canFetch() const
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
QString SyncFileInfoPrivate::sizeFormat() const
{
    if (q->isAttributes(OptInfoType::kIsDir)) {
        return QStringLiteral("-");
    }

    return FileUtils::formatSize(q->size());
}

QVariant SyncFileInfoPrivate::attribute(DFileInfo::AttributeID key, bool *ok) const
{
    auto tmp = dfmFileInfo;
    if (tmp) {
        {
            QReadLocker locker(&const_cast<SyncFileInfoPrivate *>(this)->lock);
            if (cacheAttributes.count(key) > 0) {
                if (ok)
                    *ok = true;
                return cacheAttributes.value(key);
            }
        }

        auto value = tmp->attribute(key, ok);
        return value;
    }
    return QVariant();
}

QMap<DFileInfo::AttributeExtendID, QVariant> SyncFileInfoPrivate::mediaInfo(DFileInfo::MediaType type, QList<DFileInfo::AttributeExtendID> ids)
{
    if (dfmFileInfo) {
        {
            QWriteLocker wlocker(&lock);
            mediaType = type;
            extendIDs = ids;
        }
        {
            QReadLocker rlocker(&lock);
            auto it = ids.begin();
            while (it != ids.end()) {
                if (attributesExtend.count(*it))
                    it = ids.erase(it);
                else
                    ++it;
            }
        }
        if (!ids.isEmpty())
            updateMediaInfo(type, ids);
    }

    QReadLocker rlocker(&lock);
    return attributesExtend;
}

SyncFileInfoPrivate::SyncFileInfoPrivate(SyncFileInfo *qq)
    : q(qq)
{
}

SyncFileInfoPrivate::~SyncFileInfoPrivate()
{
}

QMimeType SyncFileInfoPrivate::readMimeType(QMimeDatabase::MatchMode mode) const
{
    QUrl url = q->urlOf(UrlInfoType::kUrl);
    if (dfmbase::FileUtils::isLocalFile(url))
        return mimeDb.mimeTypeForUrl(url);
    else
        return mimeDb.mimeTypeForFile(UrlRoute::urlToPath(url),
                                      mode);
}

}
