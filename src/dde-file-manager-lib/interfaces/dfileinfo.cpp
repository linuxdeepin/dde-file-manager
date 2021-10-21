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

#include "dfileinfo.h"
#include "private/dfileinfo_p.h"
#include "app/define.h"

#include "shutil/fileutils.h"

#include "controllers/pathmanager.h"
#include "controllers/filecontroller.h"
#include "controllers/vaultcontroller.h"

#include "app/define.h"
#include "singleton.h"
#include "usershare/usersharemanager.h"
#include "deviceinfo/udisklistener.h"
#include "tag/tagmanager.h"

#include "dfileservices.h"
#include "dthumbnailprovider.h"
#include "dfileiconprovider.h"
#include "dmimedatabase.h"
#include "dabstractfilewatcher.h"
#include "dstorageinfo.h"
#include "shutil/danythingmonitorfilter.h"
#include "dfmapplication.h"
#include "dfmstandardpaths.h"

#ifdef SW_LABEL
#include "sw_label/filemanagerlibrary.h"
#endif

#include <QDateTime>
#include <QDir>
#include <QPainter>
#include <QApplication>
#include <QtConcurrent>
#include <qplatformdefs.h>
#include <ddiskmanager.h>

#include <sys/stat.h>
#include <unistd.h>

DFM_USE_NAMESPACE

#define REQUEST_THUMBNAIL_DEALY 500

class RequestEP : public QThread
{
    Q_OBJECT

public:
    static RequestEP *instance();

    ~RequestEP() override;

    // Request get the file extra properties
    QQueue<QPair<DUrl, DFileInfoPrivate *>> requestEPFiles;
    QReadWriteLock requestEPFilesLock;
    QSet<DFileInfoPrivate *> dirtyFileInfos;
    QMutex dirtyFileInfosMutex;
    QMutex requestEPCancelLock;
    bool isCanceled = false;

    void run() override;
    void requestEP(const DUrl &url, DFileInfoPrivate *info);
    void cancelRequestEP(DFileInfoPrivate *info);

Q_SIGNALS:
    void requestEPFinished(const DUrl &url, const QVariantHash &ep);

private Q_SLOTS:
    void processEPChanged(const DUrl &url, DFileInfoPrivate *info, const QVariantHash &ep);

private:
    explicit RequestEP(QObject *parent = nullptr);
};

RequestEP::RequestEP(QObject *parent)
    : QThread(parent)
{
    QMetaType::registerEqualsComparator<QList<QColor>>();
    qRegisterMetaType<DFileInfoPrivate *>();

    connect(this, &RequestEP::finished, this, [this] {
        QMutexLocker  lk(&dirtyFileInfosMutex);
        dirtyFileInfos.clear();
    });
}

RequestEP *RequestEP::instance()
{
    // RequestEP对象必须和线程相关，不然线程销毁后，QObject::thread()为nullptr
    // 导致使用QCoreApplication::postEvent给此对象发送事件无效
    // 间接导致 run 中调用  QMetaObject::invokeMethod(this, "processEPChanged", Qt::QueuedConnection, Q_ARG(DUrl, url), Q_ARG(DFileInfoPrivate*, file_info.second), Q_ARG(QVariantHash, ep));
    // 时无效，因为Qt::QueueConnection参数会将此调用交给事件循环处理
    thread_local static RequestEP eq;

    return &eq;
}

RequestEP::~RequestEP()
{
    requestEPFilesLock.lockForWrite();
    requestEPFiles.clear();
    requestEPFilesLock.unlock();

    if (!wait(1000)) {
        terminate();
    }
}

void RequestEP::run()
{
    forever {
        requestEPFilesLock.lockForRead();
        if (requestEPFiles.isEmpty()) {
            requestEPFilesLock.unlock();
            return;
        }
        requestEPFilesLock.unlock();
        requestEPFilesLock.lockForWrite();
        auto file_info = requestEPFiles.dequeue();
        requestEPFilesLock.unlock();

        const DUrl &url = file_info.first;
        const QStringList &tag_list = DFileService::instance()->getTagsThroughFiles(nullptr, {url});

        QVariantHash ep;

        if (!tag_list.isEmpty()) {
            ep["tag_name_list"] = tag_list;
        }

        QList<QColor> colors;

        for (const QColor &color : TagManager::instance()->getTagColor(tag_list)) {
            colors << color;
        }

        if (!colors.isEmpty()) {
            ep["colored"] = QVariant::fromValue(colors);
        }

        QMetaObject::invokeMethod(this, "processEPChanged", Qt::QueuedConnection,
                                  Q_ARG(DUrl, url), Q_ARG(DFileInfoPrivate *, file_info.second), Q_ARG(QVariantHash, ep));
    }
}

void RequestEP::requestEP(const DUrl &url, DFileInfoPrivate *info)
{
    requestEPFilesLock.lockForRead();

    for (int i = 0; i < requestEPFiles.count(); ++i) {
        auto file_info = requestEPFiles.at(i);

        if (file_info.second == info) {
            requestEPFilesLock.unlock();
            return;
        }
    }

    requestEPFilesLock.unlock();
    requestEPFilesLock.lockForWrite();
    requestEPFiles << qMakePair(url, info);
    requestEPFilesLock.unlock();

    if (!isRunning()) {
        start();
    }
}

void RequestEP::cancelRequestEP(DFileInfoPrivate *info)
{
    requestEPCancelLock.lock();
    isCanceled = true;
    requestEPCancelLock.unlock();

    dirtyFileInfosMutex.lock();
    dirtyFileInfos << info;
    dirtyFileInfosMutex.unlock();

    requestEPFilesLock.lockForRead();

    for (int i = 0; i < requestEPFiles.count(); ++i) {
        auto file_info = requestEPFiles.at(i);

        if (file_info.second == info) {
            requestEPFilesLock.unlock();
            requestEPFilesLock.lockForWrite();
            requestEPFiles.removeAt(i);
            requestEPFilesLock.unlock();
            info->requestEP = nullptr;

            dirtyFileInfosMutex.lock();
            dirtyFileInfos.remove(info);
            dirtyFileInfosMutex.unlock();
            return;
        }
    }

    requestEPFilesLock.unlock();
}

void RequestEP::processEPChanged(const DUrl &url, DFileInfoPrivate *info, const QVariantHash &ep)
{
    Q_EMIT requestEPFinished(url, ep);

    if (info == nullptr) {
        return;
    }

    //fix bug 44093 和 task 36486 文件的多次创建和删除崩溃
    if (isCanceled) {
        return;
    }

    requestEPCancelLock.lock();
    if (isCanceled) {
        requestEPCancelLock.unlock();
        return;
    }
    QVariantHash oldEP;

    dirtyFileInfosMutex.lock();
    if (!dirtyFileInfos.contains(info)) {
        oldEP = info->extraProperties;
        info->extraProperties = ep;
        info->epInitialized = true;
        info->requestEP = nullptr;
    } else {
        dirtyFileInfos.remove(info);
        info = nullptr;
    }
    dirtyFileInfosMutex.unlock();

    if (!ep.isEmpty() || oldEP != ep) {
        DAbstractFileWatcher::ghostSignal(url.parentUrl(), &DAbstractFileWatcher::fileAttributeChanged, url, 0); // source is internal signal

        if (info) {
            // ###(zccrs): DFileSystemModel中收到通知后会调用DAbstractFileInfo::refresh，导致会重新获取扩展属性
            info->epInitialized = true;
        }
    }
    requestEPCancelLock.unlock();
}

DFileInfoPrivate::DFileInfoPrivate(const DUrl &fileUrl, DFileInfo *qq, bool hasCache)
    : DAbstractFileInfoPrivate(fileUrl, qq, hasCache)
{
    fileInfo.setFile(fileUrl.toLocalFile());
    gvfsMountFile = 0;
}

DFileInfoPrivate::~DFileInfoPrivate()
{
    if (getIconTimer) {
        getIconTimer->disconnect(SIGNAL(timeout())); // 导致指针析构问题,UT注意
        getIconTimer->stop();
        getIconTimer->deleteLater();
    }

    if (getEPTimer) {
        getEPTimer->disconnect(SIGNAL(timeout())); // 导致指针析构问题，UT注意
        getEPTimer->stop();
        getEPTimer->deleteLater();
    }

    if (requestEP)
        requestEP->cancelRequestEP(this);
}

bool DFileInfoPrivate::isLowSpeedFile() const
{
    if (lowSpeedFile < 0) {
        lowSpeedFile = DStorageInfo::isLowSpeedDevice(fileInfo.absoluteFilePath());
    }

    return lowSpeedFile;
}

DFileInfo::DFileInfo(const QString &filePath, bool hasCache)
    : DFileInfo(DUrl::fromLocalFile(filePath), hasCache)
{
}

DFileInfo::DFileInfo(const DUrl &fileUrl, bool hasCache)
    : DAbstractFileInfo(*new DFileInfoPrivate(fileUrl, this, hasCache))
{
}

DFileInfo::DFileInfo(const QFileInfo &fileInfo, bool hasCache)
    : DFileInfo(DUrl::fromLocalFile(fileInfo.absoluteFilePath()), hasCache)
{

}

DFileInfo::~DFileInfo()
{

}

bool DFileInfo::exists(const DUrl &fileUrl)
{
    return QFileInfo::exists(fileUrl.toLocalFile());
}

QMimeType DFileInfo::mimeType(const QString &filePath, QMimeDatabase::MatchMode mode, const QString inod, const bool isgvfs)
{
    static DMimeDatabase db;
    if (isgvfs) {
        return db.mimeTypeForFile(filePath, mode, inod, isgvfs);
    }
    return db.mimeTypeForFile(filePath, mode);
}

bool DFileInfo::exists() const
{
    Q_D(const DFileInfo);

    return d->fileInfo.exists() || d->fileInfo.isSymLink();
}

bool DFileInfo::isPrivate() const
{
    Q_D(const DFileInfo);

    return FileController::privateFileMatch(d->fileInfo.absolutePath(), d->fileInfo.fileName());
}

QString DFileInfo::path() const
{
    Q_D(const DFileInfo);

    return d->fileInfo.path();
}

QString DFileInfo::filePath() const
{
    Q_D(const DFileInfo);

    return d->fileInfo.filePath();
}

QString DFileInfo::absolutePath() const
{
    Q_D(const DFileInfo);

    return d->fileInfo.absolutePath();
}

QString DFileInfo::absoluteFilePath() const
{
    Q_D(const DFileInfo);

    return d->fileInfo.absoluteFilePath();
}

QString DFileInfo::fileName() const
{
    Q_D(const DFileInfo);

    if (d->fileInfo.absoluteFilePath().endsWith(QDir::separator()))
        return QFileInfo(d->fileInfo.absolutePath()).fileName();

    return d->fileInfo.fileName();
}

QString DFileInfo::fileSharedName() const
{
    const ShareInfo &info = userShareManager->getShareInfoByPath(absoluteFilePath());

    return info.shareName();
}

QList<QIcon> DFileInfo::additionalIcon() const
{
    Q_D(const DFileInfo);

    if (d->proxy)
        return d->proxy->additionalIcon();

    QList<QIcon> icons;

    bool needEmblem = true;
    // wayland TASK-38720 修复重命名文件时，文件图标有小锁显示的问题，
    // 当为快捷方式时，有源文件文件不存在的情况，所以增加特殊判断
    if (!isSymLink() && !exists()) {
        return icons;
    }

    if (isSymLink()) {
        icons << QIcon::fromTheme("emblem-symbolic-link", DFMGlobal::instance()->standardIcon(DFMGlobal::LinkIcon));
        //链接文件不显示自定义标记
        needEmblem = false;
    }

    if (!d->gvfsMountFile) {
        if (!isWritable()) {
            icons << QIcon::fromTheme("emblem-readonly", DFMGlobal::instance()->standardIcon(DFMGlobal::LockIcon));
        }

        if (!isReadable()) {
            icons << QIcon::fromTheme("emblem-unreadable", DFMGlobal::instance()->standardIcon(DFMGlobal::UnreadableIcon));
        }
    }
    //网络文件不显示自定义标记
    else {
        needEmblem = false;
    }

    if (isShared()) {
        icons << QIcon::fromTheme("emblem-shared", DFMGlobal::instance()->standardIcon(DFMGlobal::ShareIcon));
    }

    //部分文件和目录不显示徽标
    if (needEmblem &&
            fileUrl().parentUrl().path() != "/" &&
            fileUrl().parentUrl().path() != "/data" &&
            !fileUrl().path().startsWith("/media"))
        loadFileEmblems(icons);

#ifdef SW_LABEL
    QString labelIconPath = getLabelIcon();
    if (!labelIconPath.isEmpty()) {
        icons << QIcon(labelIconPath);
    }
#endif

    return icons;
}

// 此函数高频调用，使用 DFileInfo 会降低性能
bool DFileInfo::fileIsWritable(const QString &path, uint ownerId)
{
    Q_UNUSED(ownerId)

    // 如果是root，则拥有权限
    if (DFMGlobal::isRootUser()) {
        return true;
    }

    // check user's permissions for a file
    QByteArray pathBytes(path.toLocal8Bit());
    int result = access(pathBytes.data(), W_OK);
    if (result == 0) {
        return true;
    }

    return false;
}

bool DFileInfo::isDragCompressFileFormat() const
{
    Q_D(const DFileInfo);

    // 支持".zip"结尾的，支持".7z"结尾的但不支持".tar.7z"结尾的
    return d->fileInfo.fileName().endsWith(".zip")
            || (d->fileInfo.fileName().endsWith(".7z")
                && !d->fileInfo.fileName().endsWith(".tar.7z"));
}

bool DFileInfo::canRename() const
{
    if (systemPathManager->isSystemPath(absoluteFilePath()))
        return false;
    Q_D(const DFileInfo);

    return fileIsWritable(d->fileInfo.absolutePath(), d->fileInfo.ownerId());
}

bool DFileInfo::canShare() const
{
    if (isDir() && isReadable()) {
        QStorageInfo stInfo(fileUrl().toLocalFile());
        QStorageInfo hstInfo(QDir::homePath());
        QString userPath = QDir::homePath();
        if (stInfo.device() == hstInfo.device()) { // /data/user ==> /home/user ==> /dev/sda
            userPath = stInfo.rootPath() + QString("/").append(UserShareManager::getCurrentUserName());
        }

        if (absoluteFilePath().startsWith(userPath)) {
            return true;
        }
        //fix fix task 29259,说明是共享使用true
        UDiskDeviceInfoPointer info = deviceListener->getDeviceByFilePath(filePath(), true);

        if (info) {
            if (info->getMediaType() != UDiskDeviceInfo::unknown && info->getMediaType() != UDiskDeviceInfo::network)
                return true;
        } else {
            QStringList udiskspathes = DDiskManager::resolveDeviceNode(stInfo.device(), {});
            return udiskspathes.size() > 0;
        }
    }

    return false;
}

bool DFileInfo::canFetch() const
{
    if (isPrivate())
        return false;

    return (isDir() ||
            (FileUtils::isArchive(absoluteFilePath()) && DFMApplication::instance()->genericAttribute(DFMApplication::GA_PreviewCompressFile).toBool()));
}

bool DFileInfo::canTag() const
{
#ifdef DISABLE_TAG_SUPPORT
    return false;
#endif // DISABLE_TAG_SUPPORT

    bool isFiltered = DAnythingMonitorFilter::instance()->whetherFilterCurrentPath(parentUrl().toLocalFile().toLocal8Bit());
    if (!isFiltered)
        return false;

    //压缩包内的文件和目录不支持添加tag的功能
    QString compressPath = QDir::homePath() + "/.avfs/";
    if (filePath().startsWith(compressPath))
        return false;

    //主目录不允许添加标记
    QString parentPath = parentUrl().path();
    if (parentPath == "/home" || parentPath == "/data/home")
        return false;

    //判断所有用户的固定文件夹
    QString rootPath = parentUrl().parentUrl().path();
    QString path = filePath();
    if (rootPath == "/home" || rootPath == "/data/home") {
        if (path.endsWith("/Desktop") || path.endsWith("/Videos") || path.endsWith("/Music") ||
                path.endsWith("/Pictures") || path.endsWith("/Documents") || path.endsWith("/Downloads"))
            return false;
    }

    return !systemPathManager->isSystemPath(filePath());
}

bool DFileInfo::isReadable() const
{
    if (isPrivate())
        return false;

    Q_D(const DFileInfo);

    return d->fileInfo.isReadable();
}

bool DFileInfo::isWritable() const
{
    if (isPrivate())
        return false;

    Q_D(const DFileInfo);

    return d->fileInfo.isWritable();
}

bool DFileInfo::isExecutable() const
{
    Q_D(const DFileInfo);

    return d->fileInfo.isExecutable();
}

bool DFileInfo::isHidden() const
{
    Q_D(const DFileInfo);
    // blumia: 目前不在这里读 .hidden 文件并以此返回显示状态的原因是，切换到某个目录时遍历所有文件，每个文件都会调一次 isHidden() ，而每次
    //         都重新读那个文件显然不合适，FileController 又位于没有跑事件循环的线程，也无法简单的缓存并监视文件变动。
    //         鉴于需求要的特别急，所以先使用比较脏的做法，在 Controller 里创建目录迭代器时读取一次文件，这样以达到减少不必要的读取，不需要监视
    //         文件变动，并且退出并重新进入目录时即会重新加载配置。
    return d->fileInfo.isHidden() || FileController::customHiddenFileMatch(d->fileInfo.absolutePath(), d->fileInfo.fileName());
}

bool DFileInfo::isRelative() const
{
    Q_D(const DFileInfo);

    return d->fileInfo.isRelative();
}

bool DFileInfo::isAbsolute() const
{
    Q_D(const DFileInfo);

    return d->fileInfo.isAbsolute();
}

bool DFileInfo::isShared() const
{
    return userShareManager->isShareFile(absoluteFilePath());
}

bool DFileInfo::isWritableShared() const
{
    const ShareInfo &info = userShareManager->getShareInfoByPath(absoluteFilePath());

    return info.isWritable();
}

bool DFileInfo::isAllowGuestShared() const
{
    const ShareInfo &info = userShareManager->getShareInfoByPath(absoluteFilePath());

    return info.isGuestOk();
}

bool DFileInfo::makeAbsolute()
{
    Q_D(DFileInfo);

    return d->fileInfo.makeAbsolute();
}

DAbstractFileInfo::FileType DFileInfo::fileType() const
{
    Q_D(const DFileInfo);

    // fix bug#52950 【专业版1030】【文管5.2.0.72】回收站删除指向块设备的链接文件时，删除失败
    // QT_STATBUF判断链接文件属性时，判断的是指向文件的属性，使用QFileInfo判断
    QString absoluteFilePath = d->fileInfo.absoluteFilePath();
    if (absoluteFilePath.startsWith(DFMStandardPaths::location(DFMStandardPaths::TrashFilesPath))
            && d->fileInfo.isSymLink()) {
        return RegularFile;
    }

    // Cannot access statBuf.st_mode from the filesystem engine, so we have to stat again.
    // In addition we want to follow symlinks.
    const QByteArray &nativeFilePath = QFile::encodeName(absoluteFilePath);
    QT_STATBUF statBuffer;
    if (QT_STAT(nativeFilePath.constData(), &statBuffer) == 0) {
        if (S_ISDIR(statBuffer.st_mode))
            return Directory;

        if (S_ISCHR(statBuffer.st_mode))
            return CharDevice;

        if (S_ISBLK(statBuffer.st_mode))
            return BlockDevice;

        if (S_ISFIFO(statBuffer.st_mode))
            return FIFOFile;

        if (S_ISSOCK(statBuffer.st_mode))
            return SocketFile;

        if (S_ISREG(statBuffer.st_mode))
            return RegularFile;
    }

    return Unknown;
}

bool DFileInfo::isFile() const
{
    Q_D(const DFileInfo);

    return d->fileInfo.isFile();
}

bool DFileInfo::isDir() const
{
    Q_D(const DFileInfo);

    return d->fileInfo.isDir();
}

bool DFileInfo::isSymLink() const
{
    Q_D(const DFileInfo);

    return d->fileInfo.isSymLink();
}

bool DFileInfo::canDragCompress() const
{
    Q_D(const DFileInfo);

    return isDragCompressFileFormat()
            && d->fileInfo.isWritable()
            && d->fileInfo.isReadable();
}

QString DFileInfo::symlinkTargetPath() const
{
    Q_D(const DFileInfo);

    if (d->fileInfo.isSymLink()) {
        char s[PATH_MAX + 1];
        int len = static_cast<int>(readlink(d->fileInfo.absoluteFilePath().toLocal8Bit().constData(), s, PATH_MAX));

        return QString::fromLocal8Bit(s, len);
    }

    return QString();
}

DUrl DFileInfo::symLinkTarget() const
{
    Q_D(const DFileInfo);

    if (d->fileInfo.isSymLink())
        return DUrl::fromLocalFile(d->fileInfo.symLinkTarget());

    return DAbstractFileInfo::symLinkTarget();
}

QString DFileInfo::owner() const
{
    Q_D(const DFileInfo);

    return d->fileInfo.owner();
}

uint DFileInfo::ownerId() const
{
    Q_D(const DFileInfo);

    return d->fileInfo.ownerId();
}

QString DFileInfo::group() const
{
    Q_D(const DFileInfo);

    return d->fileInfo.group();
}

uint DFileInfo::groupId() const
{
    Q_D(const DFileInfo);

    return d->fileInfo.groupId();
}

bool DFileInfo::permission(QFileDevice::Permissions permissions) const
{
    Q_D(const DFileInfo);

    if (isPrivate())
        return false;

    return d->fileInfo.permission(permissions);
}

QFileDevice::Permissions DFileInfo::permissions() const
{
    Q_D(const DFileInfo);

    if (isPrivate())
        return QFileDevice::Permissions();

    return d->fileInfo.permissions();
}

qint64 DFileInfo::size() const
{
    Q_D(const DFileInfo);

    return d->fileInfo.size();
}

int DFileInfo::filesCount() const
{

    if (isDir())
        return FileUtils::filesCount(absoluteFilePath());

    return -1;
}

QString DFileInfo::sizeDisplayName() const
{
    if (isDir()) {
        return QStringLiteral("-");
    }

    return FileUtils::formatSize(size());
}

QDateTime DFileInfo::created() const
{
    Q_D(const DFileInfo);

    return d->fileInfo.created();
}

QDateTime DFileInfo::lastModified() const
{
    Q_D(const DFileInfo);

    if (isSymLink() && !d->fileInfo.exists()) {
        struct stat attrib;

        if (lstat(d->fileInfo.filePath().toLocal8Bit().constData(), &attrib) >= 0)
            return QDateTime::fromTime_t(static_cast<uint>(attrib.st_mtime));
    }

    return d->fileInfo.lastModified();
}

QDateTime DFileInfo::lastRead() const
{
    Q_D(const DFileInfo);

    if (isSymLink() && !d->fileInfo.exists()) {
        struct stat attrib;

        if (lstat(d->fileInfo.filePath().toLocal8Bit().constData(), &attrib) >= 0)
            return QDateTime::fromTime_t(static_cast<uint>(attrib.st_mtime));
    }

    return d->fileInfo.lastRead();
}

QMimeType DFileInfo::mimeType(QMimeDatabase::MatchMode mode) const
{
    Q_D(const DFileInfo);

    if (!d->mimeType.isValid() || d->mimeTypeMode != mode) {
        //优化是苹果的就用新的minetype
        DUrl url = fileUrl();

        d->mimeType = mimeType(fileUrl().path(), mode);
        d->mimeTypeMode = mode;
    }

    return d->mimeType;
}

bool DFileInfo::canIteratorDir() const
{
    return true;
}

QString DFileInfo::subtitleForEmptyFloder() const
{
    Q_D(const DFileInfo);

    if (!exists()) {
        return QObject::tr("File has been moved or deleted");
    } else if (!isReadable()) {
        return QObject::tr("You do not have permission to access this folder");
    } else if (isDir()) {
        //当一个目录没有执行权限时，其子文件不能被遍历也就无法显示出来
        if (!isExecutable())
            return QObject::tr("You do not have permission to traverse files in it");

        if (d->gvfsMountFile) {
            // blumia: when user visiting dir as anonymous, file permission won't work
            //         if you want to check user can list a dir or not. i.e. you'll see
            //         a 700 permission with your username on it, but you can't do readdir
            //         at all.
            struct dirent *next;
            DIR *dirp = opendir(absoluteFilePath().toUtf8().constData());
            if (!dirp) return QObject::tr("You do not have permission to access this folder");
            errno = 0;
            next = readdir(dirp);
            closedir(dirp);
            if (!next && errno != 0) {
                //            qDebug() << errno;
                return QObject::tr("You do not have permission to access this folder");
            }
        }
    }

    return QObject::tr("Folder is empty");
}

QString DFileInfo::fileDisplayName() const
{
    if (systemPathManager->isSystemPath(toLocalFile())) {
        const QString &displayName = systemPathManager->getSystemPathDisplayNameByPath(filePath());

        if (!displayName.isEmpty())
            return displayName;
    } /*else if (getUDiskListener(false) && deviceListener->isDeviceFolder(toLocalFile())) {
        const UDiskDeviceInfoPointer &deviceInfo = deviceListener->getDeviceByPath(filePath());

        if (deviceInfo && !deviceInfo->fileDisplayName().isEmpty())
            return deviceInfo->fileDisplayName();
    }*/

    return fileName();
}

void DFileInfo::refresh(const bool isForce)
{
    Q_D(DFileInfo);

    Q_UNUSED(isForce)

    d->fileInfo.refresh();
    d->icon = QIcon();
    d->epInitialized = false;
    d->hasThumbnail = -1;
    d->mimeType = QMimeType();
    d->inode = 0;
}

DUrl DFileInfo::goToUrlWhenDeleted() const
{
    const QString &absFilePath = absoluteFilePath();
    if (deviceListener->isInDeviceFolder(absFilePath) ||
            absFilePath.startsWith("/run/user") // for gvfs mount point
            || absFilePath.startsWith("/media/")) {
        return DUrl(COMPUTER_ROOT);
    }

    return DAbstractFileInfo::goToUrlWhenDeleted();
}

void DFileInfo::makeToActive()
{
    Q_D(DFileInfo);

    if (!d->isLowSpeedFile())
        d->fileInfo.refresh();

    DAbstractFileInfo::makeToActive();
}

void DFileInfo::makeToInactive()
{
    Q_D(DFileInfo);

    DAbstractFileInfo::makeToInactive();

    if (d->getIconTimer) {
        d->getIconTimer->stop();
        d->getIconTimer->deleteLater();
    } else if (d->requestingThumbnail) {
        d->requestingThumbnail = false;
        DThumbnailProvider::instance()->removeInProduceQueue(d->fileInfo, DThumbnailProvider::Large);
    }

    if (d->getEPTimer) {
        d->getEPTimer->stop();
        d->getEPTimer->deleteLater();
        d->requestEP = nullptr;
        d->epInitialized = false;
    }
}

QIcon DFileInfo::fileIcon() const
{
    Q_D(const DFileInfo);

    if (!d->icon.isNull() && !d->needThumbnail && (!d->iconFromTheme || !d->icon.name().isEmpty())) {
        return d->icon;
    }

    d->iconFromTheme = false;

    const DUrl &fileUrl = this->fileUrl();

#ifdef DFM_MINIMUM
    d->hasThumbnail = 0;
#else
    if (d->hasThumbnail < 0) {
        // 如果是保险箱，直接获取图片(解决保险箱没有图标问题)
        if (fileUrl.toString().contains(VaultController::makeVaultLocalPath())) {
            d->hasThumbnail = DThumbnailProvider::instance()->hasThumbnail(d->fileInfo) && !DStorageInfo::isCdRomDevice(absoluteFilePath());
        } else {
            d->hasThumbnail = DStorageInfo::isLocalDevice(absoluteFilePath()) && DThumbnailProvider::instance()->hasThumbnail(d->fileInfo) && !DStorageInfo::isCdRomDevice(absoluteFilePath());
        }
    }
#endif
    if (d->needThumbnail || d->hasThumbnail > 0) {
        d->needThumbnail = true;

        const QIcon icon(DThumbnailProvider::instance()->thumbnailFilePath(d->fileInfo, DThumbnailProvider::Large));

        if (!icon.isNull()) {
            QPixmap pixmap = icon.pixmap(DThumbnailProvider::Large, DThumbnailProvider::Large);
            QPainter pa(&pixmap);

            pa.setPen(Qt::gray);
            pa.drawPixmap(0, 0, pixmap);
            d->icon.addPixmap(pixmap);
            d->iconFromTheme = false;
            d->needThumbnail = false;

            return d->icon;
        }

        if (d->getIconTimer) {
            QMetaObject::invokeMethod(d->getIconTimer, "start", Qt::QueuedConnection);
        } else if (isActive()) {
            QTimer *timer = new QTimer();
            const QExplicitlySharedDataPointer<DFileInfo> me(const_cast<DFileInfo *>(this));

            d->getIconTimer = timer;
            timer->setSingleShot(true);
            timer->moveToThread(qApp->thread());
            timer->setInterval(REQUEST_THUMBNAIL_DEALY);

            QObject::connect(timer, &QTimer::timeout, timer, [fileUrl, timer, me] {
                DThumbnailProvider::instance()->appendToProduceQueue(me->d_func()->fileInfo, DThumbnailProvider::Large,
                                                                     [me](const QString & path)
                {
                    DThreadUtil::runInMainThread([me, path]() {
                        if (path.isEmpty()) {
                            me->d_func()->iconFromTheme = true;
                        } else {
                            // clean old icon
                            me->d_func()->icon = QIcon();
                        }

                        me->d_func()->needThumbnail = false;
                    });

                });
                me->d_func()->requestingThumbnail = true;
                timer->deleteLater();
            });

            QMetaObject::invokeMethod(timer, "start", Qt::QueuedConnection);
        }

        if (d->icon.isNull())
            d->icon = DFileIconProvider::globalProvider()->icon(*this);

        return d->icon;
    } else {
        d->needThumbnail = false;
    }

    if (isSymLink()) {
        const DUrl &symLinkTarget = this->symLinkTarget();

        if (symLinkTarget != fileUrl) {
            const DAbstractFileInfoPointer &fileInfo = DFileService::instance()->createFileInfo(Q_NULLPTR, symLinkTarget);

            if (fileInfo) {
                d->icon = fileInfo->fileIcon();
                d->iconFromTheme = false;

                return d->icon;
            }
        }
    }

    d->icon = DFileIconProvider::globalProvider()->icon(*this);
    d->iconFromTheme = true;

    return d->icon;
}

QString DFileInfo::iconName() const
{
    if (systemPathManager->isSystemPath(absoluteFilePath()))
        return systemPathManager->getSystemPathIconNameByPath(absoluteFilePath());

    return DAbstractFileInfo::iconName();
}

QFileInfo DFileInfo::toQFileInfo() const
{
    Q_D(const DFileInfo);

    return d->fileInfo;
}

QIODevice *DFileInfo::createIODevice() const
{
    return new QFile(absoluteFilePath());
}

QVariantHash DFileInfo::extraProperties() const
{
    Q_D(const DFileInfo);

    // ensure extra properties
    if (!d->epInitialized) {
        d->epInitialized = true;

        const DUrl &url = fileUrl();

        if (!d->getEPTimer) {
            d->getEPTimer = new QTimer(qApp);
            d->getEPTimer->setSingleShot(true);
            d->getEPTimer->moveToThread(qApp->thread());
            d->getEPTimer->setInterval(REQUEST_THUMBNAIL_DEALY);
        }

        QObject::connect(d->getEPTimer, &QTimer::timeout, d->getEPTimer, [d, url] {
            d->requestEP = RequestEP::instance();

            //线程run之前先确保fileinfo未被析构时request不会被取消
            d->requestEP->requestEPCancelLock.lock();
            d->requestEP->isCanceled = false;
            d->requestEP->requestEPCancelLock.unlock();

            d->requestEP->requestEP(url, const_cast<DFileInfoPrivate *>(d));
            d->getEPTimer->deleteLater();
        });

        QMetaObject::invokeMethod(d->getEPTimer, "start", Qt::QueuedConnection);
    }

    return d->extraProperties;
}

quint64 DFileInfo::inode() const
{
    Q_D(const DFileInfo);
    if (d->inode != 0) {
        return d->inode;
    }

    struct stat statinfo;
    QByteArray pathArry = d->fileInfo.absoluteFilePath().toUtf8();
    std::string pathStd = pathArry.toStdString();
    if (stat(pathStd.c_str(), &statinfo) != 0)
        return 0;
    d->inode = statinfo.st_ino;

    return d->inode;
}

DFileInfo::DFileInfo(DFileInfoPrivate &dd)
    : DAbstractFileInfo(dd)
{

}

#include "dfileinfo.moc"
