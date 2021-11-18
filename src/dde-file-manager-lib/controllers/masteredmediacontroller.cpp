/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: dengkeyun<dengkeyun@uniontech.com>
 *             max-lv<lvwujun@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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

#include "masteredmediacontroller.h"
#include "dfmevent.h"
#include "ddiskmanager.h"
#include "ddiskdevice.h"
#include "dblockdevice.h"
#include "deviceinfo/udisklistener.h"
#include "app/define.h"
#include "models/desktopfileinfo.h"
#include "models/masteredmediafileinfo.h"
#include "dfileservices.h"
#include "dfilewatcher.h"
#include "dialogs/dialogmanager.h"
#include "dialogs/burnoptdialog.h"
#include "interfaces/dfileproxywatcher.h"
#include "private/dabstractfilewatcher_p.h"
#include "disomaster.h"
#include "shutil/fileutils.h"
#include "controllers/masteredmediacontroller_p.h"

#include <QRegularExpression>
#include <QStandardPaths>
#include <QProcess>

DFMShadowedDirIterator::DFMShadowedDirIterator(const QUrl &path, const QStringList &nameFilters, QDir::Filters filter, QDirIterator::IteratorFlags flags)
{
    DUrl durl(path);
    const QStringList &nodes = DDiskManager::resolveDeviceNode(durl.burnDestDevice(), {});
    QString udiskspath = nodes.isEmpty() ? QString() : nodes.first();
    QSharedPointer<DBlockDevice> blkdev(DDiskManager::createBlockDevice(udiskspath));
    QSharedPointer<DDiskDevice> diskdev(DDiskManager::createDiskDevice(blkdev->drive()));
    auto points = blkdev->mountPoints();
    if (!points.isEmpty()) {
        DUrl mnturl = DUrl::fromLocalFile(QString(points.front()));
        mntpoint = mnturl.toLocalFile();
    }
    while (*mntpoint.rbegin() == '/') {
        mntpoint.chop(1);
    }
    devfile = durl.burnDestDevice();
    if (diskdev->opticalBlank()) {
        //blank disc
        iterator.clear();
        stagingiterator = QSharedPointer<QDirIterator>(
                    new QDirIterator(MasteredMediaController::getStagingFile(DUrl(path)).path(),
                                     nameFilters, filter, flags)
                    );
        return;
    }
    QString realpath = mntpoint + durl.burnFilePath();
    iterator = QSharedPointer<QDirIterator>(new QDirIterator(realpath, nameFilters, filter, flags));
    stagingiterator = QSharedPointer<QDirIterator>(
                new QDirIterator(MasteredMediaController::getStagingFile(DUrl(path)).path(),
                                 nameFilters, filter, flags)
                );
}

DUrl DFMShadowedDirIterator::next()
{
    return changeSchemeUpdate(DUrl::fromLocalFile(iterator && iterator->hasNext() ? iterator->next() : (iterator = QSharedPointer<QDirIterator>(Q_NULLPTR), stagingiterator->next())));
}

bool DFMShadowedDirIterator::hasNext() const
{
    return (iterator && iterator->hasNext()) ||
            (stagingiterator && stagingiterator->hasNext());
}

QString DFMShadowedDirIterator::fileName() const
{
    return iterator ? iterator->fileName() : stagingiterator->fileName();
}

DUrl DFMShadowedDirIterator::fileUrl() const
{
    return changeScheme(DUrl::fromLocalFile(iterator ? iterator->filePath() : stagingiterator->filePath()));
}

const DAbstractFileInfoPointer DFMShadowedDirIterator::fileInfo() const
{
    DAbstractFileInfoPointer fileinfo = DAbstractFileInfoPointer(new MasteredMediaFileInfo(fileUrl()));
    return fileinfo->exists() ? fileinfo : DAbstractFileInfoPointer(); //bug 64941, DVD+RW 只擦除文件系统部分信息，而未擦除全部，有垃圾数据，所以需要判断文件的有效性
}

DUrl DFMShadowedDirIterator::url() const
{
    return changeScheme(DUrl::fromLocalFile(iterator ? iterator->path() : stagingiterator->path()));
}

DUrl DFMShadowedDirIterator::changeScheme(DUrl in) const
{
    DUrl burntmp = DUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation) + "/" + qApp->organizationName() + "/" DISCBURN_STAGING "/");
    QString stagingroot = burntmp.path() + QString(devfile).replace('/', '_');
    DUrl ret;
    QString path = in.path();
    if (burntmp.isParentOf(in)) {
        path.replace(stagingroot, devfile + "/" BURN_SEG_STAGING);
    } else {
        path.replace(mntpoint, devfile + "/" BURN_SEG_ONDISC);
    }
    ret = DUrl::fromBurnFile(path);
    if (skip.contains(ret)) {
        ret.setFragment("dup");
    }
    return ret;
}

DUrl DFMShadowedDirIterator::changeSchemeUpdate(DUrl in)
{
    DUrl ret = changeScheme(in);
    if (seen.contains(ret.burnFilePath())) {
        skip.insert(ret);
        return DUrl();
    }
    seen.insert(ret.burnFilePath());
    return ret;
}

bool MasteredMediaFileWatcherPrivate::start()
{
    return (proxyOnDisk ? proxyOnDisk->startWatcher() : true) && proxyStaging && proxyStaging->startWatcher();
}

bool MasteredMediaFileWatcherPrivate::stop()
{
    return (proxyOnDisk ? proxyOnDisk->startWatcher() : true) && proxyStaging && proxyStaging->stopWatcher();
}

bool MasteredMediaFileWatcherPrivate::handleGhostSignal(const DUrl &target, DAbstractFileWatcher::SignalType1 signal, const DUrl &url)
{
    Q_Q(MasteredMediaFileWatcher);
    Q_UNUSED(url);

    if (target.burnDestDevice() != q->fileUrl().burnDestDevice()) {
        return false;
    }

    (q->*signal)(q->fileUrl());
    return true;
}

MasteredMediaFileWatcher::MasteredMediaFileWatcher(const DUrl &url, QObject *parent)
    : DAbstractFileWatcher(*new MasteredMediaFileWatcherPrivate(this), url, parent)
{
    Q_D(MasteredMediaFileWatcher);

    DUrl url_staging = MasteredMediaController::getStagingFile(url);
    d->proxyStaging = QPointer<DAbstractFileWatcher>(new DFileProxyWatcher(url_staging,
                                                                           new DFileWatcher(url_staging.path()),
    [](const DUrl & in)->DUrl {
        QRegularExpressionMatch m;
        QString cachepath = QRegularExpression::escape(QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation) + "/" + qApp->organizationName() + "/" DISCBURN_STAGING "/");
        m = QRegularExpression(cachepath + "(.*)").match(in.path());
        Q_ASSERT(m.hasMatch());
        QString cpth = m.captured(1);
        m = QRegularExpression("(.*?)/(.*)").match(cpth);
        QString devid(m.captured(1));
        QString path(m.captured(2));
        if (!m.hasMatch())
        {
            devid = cpth;
        }
        return DUrl::fromBurnFile(devid.replace('_', '/') + "/" BURN_SEG_STAGING "/" + path);
    }
                                                                          ));
    d->proxyStaging->moveToThread(thread());
    d->proxyStaging->setParent(this);

    connect(d->proxyStaging, &DAbstractFileWatcher::fileAttributeChanged, this, &MasteredMediaFileWatcher::onFileAttributeChanged);
    connect(d->proxyStaging, &DAbstractFileWatcher::fileDeleted, this, &MasteredMediaFileWatcher::onFileDeleted);
    connect(d->proxyStaging, &DAbstractFileWatcher::fileMoved, this, &MasteredMediaFileWatcher::onFileMoved);
    connect(d->proxyStaging, &DAbstractFileWatcher::subfileCreated, this, &MasteredMediaFileWatcher::onSubfileCreated);

    d->proxyOnDisk.clear();

    const QStringList &nodes = DDiskManager::resolveDeviceNode(url.burnDestDevice(), {});
    QString udiskspath = nodes.isEmpty() ? QString() : nodes.first();
    QSharedPointer<DBlockDevice> blkdev(DDiskManager::createBlockDevice(udiskspath));

    if (blkdev && blkdev->mountPoints().size()) {
        DUrl url_mountpoint = DUrl::fromLocalFile(blkdev->mountPoints().front());
        d->proxyOnDisk = QPointer<DAbstractFileWatcher>(new DFileWatcher(url_mountpoint.path()));
        d->proxyOnDisk->moveToThread(thread());
        d->proxyOnDisk->setParent(this);
        connect(d->proxyOnDisk, &DAbstractFileWatcher::fileDeleted, this, [this, url, url_mountpoint](DUrl deletedUrl) {
            // 光盘挂载点中的文件可能被删除 (如:wps 打开文件时会创建一个临时文件, 关闭时删除) 导致此信号被触发
            // 此时若使用 url 会导致意外跳转主目录
            QString mountPath = url_mountpoint.toLocalFile();
            QString deletedPath = deletedUrl.toLocalFile();
            if (QUrl(mountPath) == QUrl(deletedPath)) {
                emit fileDeleted(url);
            } else {
                emit fileDeleted(deletedUrl);
            }
        });
    }
    /*
     * blank disc doesn't mount
     * ejecting disc by pressing the eject button doesn't properly remove the mount point
     * therefore this is always needed as a "last resort".
     */
    d->diskm.reset(new DDiskManager(this));
    connect(d->diskm.data(), &DDiskManager::opticalChanged, this,
    [this, blkdev, url](const QString & path) {
        if (path == blkdev->drive()) {
            emit fileDeleted(url);
        }
    });
    d->diskm->setWatchChanges(true);

}

void MasteredMediaFileWatcher::onFileDeleted(const DUrl &url)
{
    emit fileDeleted(url);
}

void MasteredMediaFileWatcher::onFileAttributeChanged(const DUrl &url)
{
    emit fileAttributeChanged(url);
}

void MasteredMediaFileWatcher::onFileMoved(const DUrl &fromUrl, const DUrl &toUrl)
{
    emit fileMoved(fromUrl, toUrl);
}

void MasteredMediaFileWatcher::onSubfileCreated(const DUrl &url)
{
    emit subfileCreated(url);
}

MasteredMediaController::MasteredMediaController(QObject *parent) : DAbstractFileController(parent)
{

}

bool MasteredMediaController::openFile(const QSharedPointer<DFMOpenFileEvent> &event) const
{
    DUrl url = DUrl::fromLocalFile(MasteredMediaFileInfo(event->url()).extraProperties()["mm_backer"].toString());

    return fileService->openFile(event->sender(), url);
}

bool MasteredMediaController::openFileByApp(const QSharedPointer<DFMOpenFileByAppEvent> &event) const
{
    DUrl url = DUrl::fromLocalFile(MasteredMediaFileInfo(event->url()).extraProperties()["mm_backer"].toString());

    return fileService->openFileByApp(event->sender(), event->appName(), url);
}

bool MasteredMediaController::openFilesByApp(const QSharedPointer<DFMOpenFilesByAppEvent> &event) const
{
    DUrlList lst;
    for (auto &i : event->urlList()) {
        if (i.burnIsOnDisc()) {
            DUrl transUrl = DUrl::fromLocalFile(MasteredMediaFileInfo(i).extraProperties()["mm_backer"].toString());
            lst.append(transUrl);
        }
    }
    return fileService->openFilesByApp(event->sender(), event->appName(), lst, event->isEnter());
}

bool MasteredMediaController::compressFiles(const QSharedPointer<DFMCompressEvent> &event) const
{
    DUrlList lst;
    for (auto &i : event->urlList()) {
        if (i.burnIsOnDisc()) {
            DUrl local_file = DUrl::fromLocalFile(MasteredMediaFileInfo(i).extraProperties()["mm_backer"].toString());
            lst.push_back(local_file);
        }
    }

    return fileService->compressFiles(event->sender(), lst);
}

bool MasteredMediaController::decompressFile(const QSharedPointer<DFMDecompressEvent> &event) const
{
    DUrlList lst;
    for (auto &i : event->urlList()) {
        if (i.burnIsOnDisc()) {
            DUrl local_file = DUrl::fromLocalFile(MasteredMediaFileInfo(i).extraProperties()["mm_backer"].toString());
            lst.push_back(local_file);
        }
    }

    return fileService->decompressFile(event->sender(), lst);
}

bool MasteredMediaController::deleteFiles(const QSharedPointer<DFMDeleteEvent> &event) const
{
    DUrlList lst;
    for (auto &i : event->urlList()) {
        if (!i.burnIsOnDisc()) {
            lst.push_back(getStagingFile(i));
        }
    }

    return fileService->deleteFiles(event->sender(), lst, false, event->silent());
}

DUrlList MasteredMediaController::moveToTrash(const QSharedPointer<DFMMoveToTrashEvent> &event) const
{
    DUrlList lst, retlst;
    for (auto &i : event->urlList()) {
        if (!i.burnIsOnDisc()) {
            lst.push_back(getStagingFile(i));
            retlst.push_back(i);
        }
    }
    fileService->deleteFiles(event->sender(), lst, false);

    return retlst;
}

bool MasteredMediaController::writeFilesToClipboard(const QSharedPointer<DFMWriteUrlsToClipboardEvent> &event) const
{
    //TODO: hide/disable the menu item if no selected file is already on disc.
    DUrlList lst;
    for (auto &i : event->urlList()) {
        DAbstractFileInfoPointer fp = fileService->createFileInfo(event->sender(), i);
        if (!DUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation) + "/" + qApp->organizationName() + "/" DISCBURN_STAGING "/").isParentOf(
                    DUrl::fromLocalFile(fp->extraProperties()["mm_backer"].toString()))) {
            lst.push_back(DUrl::fromLocalFile(fp->extraProperties()["mm_backer"].toString()));
        }
    }
    DFMGlobal::setUrlsToClipboard(DUrl::toQUrlList(lst), event->action());

    return !lst.empty();
}

DUrlList MasteredMediaController::pasteFile(const QSharedPointer<DFMPasteEvent> &event) const
{
    DUrlList src = event->urlList();
    DUrl dst = event->targetUrl();

    if (src.size() == 1) {
        QString dev(dst.burnDestDevice());
        bool is_blank = ISOMaster->getDevicePropertyCached(dev).formatted;
        if (!ISOMaster->getDevicePropertyCached(dev).devid.length()) {
            auto nodes = DDiskManager::resolveDeviceNode(dev, {});
            if (nodes.isEmpty())
                return DUrlList();
            QString udiskspath = nodes.first();
            QScopedPointer<DBlockDevice> blk(DDiskManager::createBlockDevice(udiskspath));
            if (!blk)
                return DUrlList();
            QScopedPointer<DDiskDevice> drv(DDiskManager::createDiskDevice(blk->drive()));
            if (!drv)
                return DUrlList();
            is_blank = drv->opticalBlank();
        }
        QString dstdirpath = getStagingFile(DUrl::fromBurnFile(dev + "/" BURN_SEG_STAGING)).path();
        QDir dstdir = QDir(dstdirpath);
        dstdir.setFilter(QDir::Filter::AllEntries | QDir::Filter::NoDotAndDotDot);
        DAbstractFileInfoPointer fi = fileService->createFileInfo(event->sender(), src.front());
        QSet<QString> image_types = {"application/x-cd-image", "application/x-iso9660-image"};
        if (is_blank && fi && image_types.contains(fi->mimeTypeName()) && dstdir.count() == 0) {
            int r = DThreadUtil::runInMainThread(dialogManager, &DialogManager::showOpticalImageOpSelectionDialog, DFMUrlBaseEvent(event->sender(), dst));
            if (r == 1) {
                DAbstractFileInfoPointer srcInfo = DFileService::instance()->createFileInfo(nullptr, src.at(0));
                qint64 srcSize = srcInfo->size();
                DISOMasterNS::DeviceProperty dp = ISOMaster->getDevicePropertyCached(dst.burnDestDevice());
                if (dp.devid.isEmpty()) {
                    ISOMaster->acquireDevice(dev);
                    dp = ISOMaster->getDeviceProperty();
                    qInfo() << "No cache for " << dev << ", acquire device again. after acquire the capacity is: " << dp.avail;
                }
                // 光盘容量小于刻录项目，对话框提示：目标磁盘剩余空间不足，无法进行刻录！
                if (dp.avail == 0 || static_cast<quint64>(srcSize) > dp.avail) {
                    DThreadUtil::runInMainThread([] {
                        dialogManager->showMessageDialog(DialogManager::msgWarn, tr("Unable to burn. Not enough free space on the target disk."));
                    });
                } else {
                    DThreadUtil::runInMainThread([src, dev] {
                        QScopedPointer<BurnOptDialog> bd(new BurnOptDialog(dev));
                        bd->setISOImage(src.front());
                        bd->exec();
                    });
                }
                return DUrlList{};
            }
            if (r == 0 || r == -1) {
                return DUrlList{};
            }
        }
    }

    DUrl tmpdst = getStagingFile(dst); // getStagingFile 返回 file的path，叫getStagingfile比较恰当
    QFileInfo fileInfo(tmpdst.path());
    if(fileInfo.isFile()) {
        tmpdst = tmpdst.parentUrl();
    }

    FileUtils::mkpath(tmpdst);

    return fileService->pasteFile(event->sender(), event->action(), tmpdst, src);
}

const DAbstractFileInfoPointer MasteredMediaController::createFileInfo(const QSharedPointer<DFMCreateFileInfoEvent> &event) const
{
    return DAbstractFileInfoPointer(new MasteredMediaFileInfo(event->url()));
}

const DDirIteratorPointer MasteredMediaController::createDirIterator(const QSharedPointer<DFMCreateDiriterator> &event) const
{
    //Make sure the staging folder exists. Otherwise the staging watcher won't work.
    if (event->url().burnFilePath().contains(QRegularExpression("^/*$"))) {
        // 不走文管的事件机制创建 staging folter, 不需要撤销处理
        DUrl &&stagingUrl = getStagingFile(event->url());
        if (!QDir().mkpath(stagingUrl.toLocalFile())) {
            FileUtils::mkpath(stagingUrl);
        }
    }
    return DDirIteratorPointer(new DFMShadowedDirIterator(event->url(), event->nameFilters(), event->filters(), event->flags()));
}

bool MasteredMediaController::shareFolder(const QSharedPointer<DFMFileShareEvent> &event) const
{
    DUrl url = DUrl::fromLocalFile(MasteredMediaFileInfo(event->url()).extraProperties()["mm_backer"].toString());

    return fileService->shareFolder(event->sender(), url, event->name(), event->isWritable(), event->allowGuest());
}

bool MasteredMediaController::unShareFolder(const QSharedPointer<DFMCancelFileShareEvent> &event) const
{
    DUrl url = DUrl::fromLocalFile(MasteredMediaFileInfo(event->url()).extraProperties()["mm_backer"].toString());

    return fileService->unShareFolder(event->sender(), url);
}

bool MasteredMediaController::openInTerminal(const QSharedPointer<DFMOpenInTerminalEvent> &event) const
{
    if (!event->url().burnIsOnDisc()) {
        return false;
    }

    const QString &current_dir = QDir::currentPath();

    QString backer = MasteredMediaFileInfo(event->url()).extraProperties()["mm_backer"].toString();
    if (!backer.length()) {
        return false;
    }
    QDir::setCurrent(backer);

    bool ok = QProcess::startDetached(FileUtils::defaultTerminalPath());

    QDir::setCurrent(current_dir);

    return ok;
}

bool MasteredMediaController::createSymlink(const QSharedPointer<DFMCreateSymlinkEvent> &event) const
{
    if (!event->fileUrl().burnIsOnDisc()) {
        return false;
    }

    if (event->toUrl().scheme() == BURN_SCHEME) {
        return false;
    }

    DUrl local_url = DUrl::fromLocalFile(MasteredMediaFileInfo(event->fileUrl()).extraProperties()["mm_backer"].toString());
    return fileService->createSymlink(event->sender(), local_url, event->toUrl(), true);
}

bool MasteredMediaController::addToBookmark(const QSharedPointer<DFMAddToBookmarkEvent> &event) const
{
    DUrl destUrl = event->url();

    const DAbstractFileInfoPointer &p = fileService->createFileInfo(nullptr, destUrl);
    DUrl bookmarkUrl = DUrl::fromBookMarkFile(destUrl, p->fileDisplayName());

    return fileService->touchFile(event->sender(), bookmarkUrl);
}

bool MasteredMediaController::removeBookmark(const QSharedPointer<DFMRemoveBookmarkEvent> &event) const
{
    return DFileService::instance()->deleteFiles(nullptr, {DUrl::fromBookMarkFile(event->url(), QString())}, false);
}

DAbstractFileWatcher *MasteredMediaController::createFileWatcher(const QSharedPointer<DFMCreateFileWatcherEvent> &event) const
{
    return new MasteredMediaFileWatcher(event->url());
}


DUrl MasteredMediaController::getStagingFile(DUrl dst)
{
    Q_ASSERT(dst.burnDestDevice().length() > 0);
    return DUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation)
                               + "/" + qApp->organizationName() + "/" DISCBURN_STAGING "/"
                               + dst.burnDestDevice().replace('/', '_')
                               + dst.burnFilePath());
}

DUrl MasteredMediaController::getStagingFile(QString dev)
{
    return DUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation)   // ~/.cache
                  + "/" + qApp->organizationName() + "/" DISCBURN_STAGING "/"                           // ~/.cache/deepin/discburn/
                  + dev.replace('/','_'));                                                              // ~/.cache/deepin/discburn/_dev_srN
}

QFileDevice::Permissions MasteredMediaController::getPermissionsCopyToLocal()
{
    // 基础的 rw-rw-r-- 权限
    static const QFileDevice::Permissions permissionsToLocal = (QFileDevice::WriteUser | QFileDevice::ReadUser
                                                                | QFileDevice::WriteGroup | QFileDevice::ReadGroup
                                                                | QFileDevice::ReadOther);
    return permissionsToLocal;
}
