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

//fix:修正临时拷贝文件到光盘的路径问题，不是挂载目录，而是临时缓存目录
#include "dfilemenumanager.h"

#include "appcontroller.h"
#include "trashjobcontroller.h"
#include "deletejobcontroller.h"
#include "filecontroller.h"
#include "trashmanager.h"
#include "searchcontroller.h"
#include "sharecontroler.h"
#include "avfsfilecontroller.h"
#include "mountcontroller.h"
#include "masteredmediacontroller.h"
#include "vaultcontroller.h"
#include "bookmarkmanager.h"
#include "networkcontroller.h"
#include "mergeddesktopcontroller.h"
#include "dfmrootcontroller.h"
#include "dfmappentrycontroller.h"
#include "deviceinfo/udisklistener.h"
#include "dfileservices.h"
#include "fileoperations/filejob.h"
#include "dfmeventdispatcher.h"
#include "dfmapplication.h"
#include "disomaster.h"
#include "pathmanager.h"

#include "app/filesignalmanager.h"
#include "dfmevent.h"
#include "app/define.h"

#include "interfaces/dfmstandardpaths.h"
#include "interfaces/defenderinterface.h"
#include "interfaces/dumountmanager.h"
#include "shutil/fileutils.h"
#include "views/windowmanager.h"
#include "views/dfilemanagerwindow.h"
#include "views/dtagedit.h"
#include "views/dfmopticalmediawidget.h"

#include "gvfs/networkmanager.h"
#include "gvfs/gvfsmountmanager.h"
#include "gvfs/secretmanager.h"
#include "usershare/usersharemanager.h"
#include "dialogs/dialogmanager.h"
#include "singleton.h"

#include "tagcontroller.h"
#include "recentcontroller.h"
#include "views/drenamebar.h"
#include "shutil/filebatchprocess.h"
#include "utils.h"

#include "../deviceinfo/udisklistener.h"

#include <QProcess>
#include <QStorageInfo>
#include <QAction>
#include <DAboutDialog>
#include <qprocess.h>
#include <QtConcurrent>
#include <QMutex>

#include <DApplication>
#include <DDesktopServices>

#include "tag/tagutil.h"
#include "tag/tagmanager.h"
#include "shutil/shortcut.h"
//#include "views/dbookmarkitem.h"
#include "models/desktopfileinfo.h"
#include "models/dfmrootfileinfo.h"
#include "controllers/tagmanagerdaemoncontroller.h"

#include "dblockdevice.h"
#include "ddiskdevice.h"
#include "ddiskmanager.h"
#include "dgiovolumemanager.h"
#include "dgiofile.h"
#include "dgiomount.h"

#ifdef SW_LABEL
#include "sw_label/filemanagerlibrary.h"
#endif

using namespace Dtk::Widget;

///###: be used for tag protocol.
template<typename Ty>
using iterator = typename QList<Ty>::iterator;

template<typename Ty>
using citerator = typename QList<Ty>::const_iterator;

const char *empty_recent_file =
    R"|(<?xml version="1.0" encoding="UTF-8"?>
        <xbel version="1.0"
        xmlns:bookmark="http://www.freedesktop.org/standards/desktop-bookmarks"
        xmlns:mime="http://www.freedesktop.org/standards/shared-mime-info"
        >
        </xbel>)|";

QPair<DUrl, quint64> AppController::selectionAndRenameFile;
QPair<DUrl, quint64> AppController::selectionFile;
QPair<QList<DUrl>, quint64> AppController::multiSelectionFilesCache;
std::atomic<quint64> AppController::multiSelectionFilesCacheCounter{ 0 };
std::atomic<bool> AppController::flagForDDesktopRenameBar{ false };


class AppController_ : public AppController {};
Q_GLOBAL_STATIC(AppController_, acGlobal)

AppController *AppController::instance()
{
    return acGlobal;
}

void AppController::registerUrlHandle()
{
    DFileService::dRegisterUrlHandler<FileController>(FILE_SCHEME, "");
    //    DFileService::dRegisterUrlHandler<TrashManager>(TRASH_SCHEME, "");
    //sanitinizer工具检测到DFileService::setFileUrlHandler(TRASH_SCHEME, "", new TrashManager());泄露
    auto tempTrashMgr = new TrashManager();
    tempTrashMgr->setObjectName("trashMgr");
    DFileService::setFileUrlHandler(TRASH_SCHEME, "", tempTrashMgr);
    DFileService::dRegisterUrlHandler<SearchController>(SEARCH_SCHEME, "");
    DFileService::dRegisterUrlHandler<NetworkController>(NETWORK_SCHEME, "");
    DFileService::dRegisterUrlHandler<NetworkController>(SMB_SCHEME, "");
    DFileService::dRegisterUrlHandler<NetworkController>(SFTP_SCHEME, "");
    DFileService::dRegisterUrlHandler<NetworkController>(FTP_SCHEME, "");
    DFileService::dRegisterUrlHandler<NetworkController>(DAV_SCHEME, "");
    DFileService::dRegisterUrlHandler<ShareControler>(USERSHARE_SCHEME, "");
    DFileService::dRegisterUrlHandler<AVFSFileController>(AVFS_SCHEME, "");
    DFileService::dRegisterUrlHandler<MountController>(MOUNT_SCHEME, "");
    DFileService::dRegisterUrlHandler<MasteredMediaController>(BURN_SCHEME, "");

    DFileService::dRegisterUrlHandler<TagController>(TAG_SCHEME, "");
    DFileService::dRegisterUrlHandler<RecentController>(RECENT_SCHEME, "");
    DFileService::dRegisterUrlHandler<MergedDesktopController>(DFMMD_SCHEME, "");
    DFileService::dRegisterUrlHandler<DFMRootController>(DFMROOT_SCHEME, "");
    DFileService::dRegisterUrlHandler<VaultController>(DFMVAULT_SCHEME, "");
    DFileService::dRegisterUrlHandler<DFMAppEntryController>(APPENTRY_SCHEME, "");
}

void AppController::actionOpen(const QSharedPointer<DFMUrlListBaseEvent> &event, const bool isEnter)
{
    const DUrlList &urls = event->urlList();
    if (urls.isEmpty()) {
        return;
    }

    if (urls.size() > 1 || DFMApplication::instance()->appAttribute(DFMApplication::AA_AllwayOpenOnNewWindow).toBool()) {
        // 选择多文件打开的时候调用函数 openFiles，单文件打开调用 openFile，部分 controller 没有实现 openFiles 函数，因此在这里转换成 file:// 的 url，
        // 通过 fileController 来进行打开调用
        DUrlList lstUrls;
        for (DUrl u : urls) {
            if (u.scheme() == RECENT_SCHEME) {
                u = DUrl::fromLocalFile(u.path());
            } else if (u.scheme() == SEARCH_SCHEME) { //搜索结果也存在右键批量打卡不成功的问题，这里做类似处理
                u = u.searchedFileUrl();
            } else if (u.scheme() == BURN_SCHEME) {
                DAbstractFileInfoPointer info = fileService->createFileInfo(event->sender(), u);
                if (!info)
                    continue;
                if (info->canRedirectionFileUrl())
                    u = info->redirectedFileUrl();
            }
            lstUrls << u;
        }

        //! bug 38585 判断是不是保险箱以及是否是解锁状态，如果是就发送processEventAsync事件OpenInCurrentWindow走解锁流程
        if (VaultController::ins()->state() != VaultController::Unlocked && lstUrls.size() == 1 && lstUrls.first().isVaultFile()) {
            DFMEventDispatcher::instance()->processEventAsync<DFMOpenUrlEvent>(event->sender(), urls, DFMOpenUrlEvent::OpenInCurrentWindow, isEnter);
        } else {
            DFMEventDispatcher::instance()->processEvent<DFMOpenUrlEvent>(event->sender(), lstUrls, DFMOpenUrlEvent::ForceOpenNewWindow, isEnter);
        }
    } else {
        //fix bug 30506 ,异步处理网路文件很卡的情况下，快速点击会崩溃，或者卡死,使用fileinfo中的是否是gvfsmount
        if (urls.size() > 0 && DFileService::instance()->createFileInfo(nullptr, urls.first())->isGvfsMountFile()/*FileUtils::isGvfsMountFile(urls.first().path())*/) {
            DFMEventDispatcher::instance()->processEvent<DFMOpenUrlEvent>(event->sender(), urls, DFMOpenUrlEvent::OpenInCurrentWindow);
            return;
        }
        DFMEventDispatcher::instance()->processEventAsync<DFMOpenUrlEvent>(event->sender(), urls, DFMOpenUrlEvent::OpenInCurrentWindow);
    }
}

void AppController::actionOpenDisk(const QSharedPointer<DFMUrlBaseEvent> &event)
{
    const DUrl &fileUrl = event->url();
    if (!fileUrl.isValid()) {
        return;
    }

    bool mounted = QStorageInfo(fileUrl.toLocalFile()).isValid();

    DAbstractFileInfoPointer fi = fileService->createFileInfo(event->sender(), fileUrl);
    if (fi && fi->scheme() == DFMROOT_SCHEME) {
        mounted |= (!fi->redirectedFileUrl().isEmpty());
    }

    QScopedPointer<DBlockDevice> blk(DDiskManager::createBlockDevice(fi->extraProperties()["udisksblk"].toString()));
    QScopedPointer<DDiskDevice> drv(DDiskManager::createDiskDevice(blk->drive()));

    if (fileUrl.path().contains("dfmroot:///sr") && blk->idUUID().isEmpty() && !drv->opticalBlank()) return; // 如果光驱的uuid为空（光盘未挂载）且不是空光盘的情况下

    if (!mounted) {
        m_fmEvent = event;
        setEventKey(Open);
        actionMount(event);
        deviceListener->addSubscriber(this);
    } else {
        DUrl newUrl = fileUrl;
        newUrl.setQuery(QString());

        if (fi && fi->scheme() == DFMROOT_SCHEME) {
            newUrl = fi->redirectedFileUrl();
        }

        const QSharedPointer<DFMUrlListBaseEvent> &e = dMakeEventPointer<DFMUrlListBaseEvent>(event->sender(), DUrlList() << newUrl);
        e->setWindowId(event->windowId());
        actionOpen(e);
    }
}


void AppController::asyncOpenDisk(const QString &path)
{
    DUrlList urls;
    urls << DUrl(path);
    m_fmEvent->setData(urls);
    actionOpen(m_fmEvent.staticCast<DFMUrlListBaseEvent>());
}

void AppController::actionOpenInNewWindow(const QSharedPointer<DFMUrlListBaseEvent> &event)
{
    DFMEventDispatcher::instance()->processEvent<DFMOpenNewWindowEvent>(event->sender(), event->urlList(), true);
}

void AppController::actionOpenInNewTab(const QSharedPointer<DFMUrlBaseEvent> &event)
{
    DFMEventDispatcher::instance()->processEvent<DFMOpenNewTabEvent>(event->sender(), event->url());
}

void AppController::actionOpenDiskInNewTab(const QSharedPointer<DFMUrlBaseEvent> &event)
{
    const DUrl &fileUrl = event->url();

    if (!fileUrl.isValid()) {
        return;
    }

    bool mounted = QStorageInfo(fileUrl.toLocalFile()).isValid();

    DAbstractFileInfoPointer fi = fileService->createFileInfo(event->sender(), fileUrl);
    if (fi && fi->scheme() == DFMROOT_SCHEME) {
        mounted |= (!fi->redirectedFileUrl().isEmpty());
    }

    if (!mounted) {
        m_fmEvent = event;
        actionMount(event);
        setEventKey(OpenNewTab);
        deviceListener->addSubscriber(this);
    } else {
        //FIXME(zccrs): 为了菜单中能卸载/挂载U盘，url中被设置了query字段，今后应该将此字段移动到继承的FMEvent中
        DUrl newUrl = fileUrl;

        newUrl.setQuery(QString());

        if (fi && fi->scheme() == DFMROOT_SCHEME) {
            newUrl = fi->redirectedFileUrl();
        }

        actionOpenInNewTab(dMakeEventPointer<DFMUrlBaseEvent>(event->sender(), newUrl));
    }
}

void AppController::asyncOpenDiskInNewTab(const QString &path)
{
    m_fmEvent->setData(DUrl(path));
    actionOpenDiskInNewTab(m_fmEvent.staticCast<DFMUrlBaseEvent>());
}

void AppController::actionOpenDiskInNewWindow(const QSharedPointer<DFMUrlBaseEvent> &event)
{
    const DUrl &fileUrl = event->url();

    if (!fileUrl.isValid()) {
        return;
    }

    bool mounted = QStorageInfo(fileUrl.toLocalFile()).isValid();

    DAbstractFileInfoPointer fi = fileService->createFileInfo(event->sender(), fileUrl);
    if (fi && fi->scheme() == DFMROOT_SCHEME) {
        mounted |= (!fi->redirectedFileUrl().isEmpty());
    }

    if (!mounted) {
        m_fmEvent = event;
        actionMount(event);
        setEventKey(OpenNewWindow);
        deviceListener->addSubscriber(this);
    } else {
        //FIXME(zccrs): 为了菜单中能卸载/挂载U盘，url中被设置了query字段，今后应该将此字段移动到继承的FMEvent中
        DUrl newUrl = fileUrl;

        newUrl.setQuery(QString());

        if (fi && fi->scheme() == DFMROOT_SCHEME) {
            newUrl = fi->redirectedFileUrl();
        }

        const QSharedPointer<DFMUrlListBaseEvent> newEvent(new DFMUrlListBaseEvent(event->sender(), DUrlList() << newUrl));

        newEvent->setWindowId(event->windowId());

        actionOpenInNewWindow(newEvent);
    }
}

void AppController::asyncOpenDiskInNewWindow(const QString &path)
{
    DUrlList urls;
    urls << DUrl(path);
    m_fmEvent->setData(urls);
    actionOpenInNewWindow(m_fmEvent.staticCast<DFMUrlListBaseEvent>());
}

void AppController::actionOpenAsAdmin(const QSharedPointer<DFMUrlBaseEvent> &event)
{
    QStringList args;
    // fix bug#42601 【专业版 sp3】【文件管理器】【5.2.0.21-1】搜索结果页，右键文件夹点击以管理员身份打开，新窗口未聚焦到对应文件夹
    args << event->url().toLocalFile();
    qDebug() << args;
    QProcess::startDetached("dde-file-manager-pkexec", args);
}

void AppController::actionOpenWithCustom(const QSharedPointer<DFMUrlBaseEvent> &event)
{
    emit fileSignalManager->requestShowOpenWithDialog(DFMUrlBaseEvent(event->sender(), event->url()));
}

//新加app打开多个url
void AppController::actionOpenFilesWithCustom(const QSharedPointer<DFMUrlListBaseEvent> &event)
{
    emit fileSignalManager->requestShowOpenFilesWithDialog(DFMUrlListBaseEvent(event->sender(), event->urlList()));
}

void AppController::actionOpenFileLocation(const QSharedPointer<DFMUrlListBaseEvent> &event)
{
    const DUrlList &urls = event->urlList();
    foreach (const DUrl &url, urls) {
        fileService->openFileLocation(event->sender(), url);
    }
}


void AppController::actionCompress(const QSharedPointer<DFMUrlListBaseEvent> &event)
{
    fileService->compressFiles(event->sender(), event->urlList());
}

void AppController::actionDecompress(const QSharedPointer<DFMUrlListBaseEvent> &event)
{
    fileService->decompressFile(event->sender(), event->urlList());
}

void AppController::actionDecompressHere(const QSharedPointer<DFMUrlListBaseEvent> &event)
{
    fileService->decompressFileHere(event->sender(), event->urlList());
}

void AppController::actionCut(const QSharedPointer<DFMUrlListBaseEvent> &event)
{
    fileService->writeFilesToClipboard(event->sender(), DFMGlobal::CutAction, event->urlList());

}

void AppController::actionCopy(const QSharedPointer<DFMUrlListBaseEvent> &event)
{
    fileService->writeFilesToClipboard(event->sender(), DFMGlobal::CopyAction, event->urlList());
}

void AppController::actionPaste(const QSharedPointer<DFMUrlBaseEvent> &event)
{
    // QTimer::singleshot目的: 多窗口下粘贴，右键拖动标题栏
    QTimer::singleShot(0, [event]() {
        fileService->pasteFileByClipboard(event->sender(), event->url());
    });
}

void AppController::actionRename(const QSharedPointer<DFMUrlListBaseEvent> &event)
{
    DUrlList urlList{ event->urlList() };
    if (urlList.size() == 1) { //###: for one file.
        QSharedPointer<DFMUrlBaseEvent> singleFileEvent{ dMakeEventPointer<DFMUrlBaseEvent>(event->sender(), urlList.first()) };
        emit fileSignalManager->requestRename(*singleFileEvent);

    } else { //###: for more than one file.
        emit fileSignalManager->requestMultiFilesRename(*event);
    }
}

void AppController::actionBookmarkRename(const QSharedPointer<DFMUrlBaseEvent> &event)
{
    emit fileSignalManager->requestBookmarkRename(*event.data());
}

void AppController::actionBookmarkRemove(const QSharedPointer<DFMUrlBaseEvent> &event)
{
    fileService->removeBookmark(event->sender(), event->url());
}

void AppController::actionDelete(const QSharedPointer<DFMUrlListBaseEvent> &event)
{
    // 使用 Qtimer 避免回收站有大量文件时，异步事件处理时间过长，导致窗口不能拖动
    QTimer::singleShot(1, [event]() {
        fileService->moveToTrash(event->sender(), event->urlList());
    });
}

void AppController::actionCompleteDeletion(const QSharedPointer<DFMUrlListBaseEvent> &event)
{
    auto &&list = event->urlList();
    if (list.isEmpty())
        return;

    auto sender { event->sender() };
    foreach (const DUrl &url, list) {
        if (systemPathManager->isSystemPath(url.toLocalFile())) {
            DThreadUtil::runInMainThread(dialogManager, &DialogManager::showDeleteSystemPathWarnDialog, DFMEvent::windowIdByQObject(sender));
            return;
        }
    }

    bool slient { false };
    bool force { false };
    DFMEventDispatcher::instance()->processEventAsync(dMakeEventPointer<DFMDeleteEvent>(sender, list, slient, force));
}

void AppController::actionCreateSymlink(const QSharedPointer<DFMUrlBaseEvent> &event)
{
    fileService->createSymlink(event->sender(), event->url());
}

void AppController::actionSendToDesktop(const QSharedPointer<DFMUrlListBaseEvent> &event)
{
    fileService->sendToDesktop(event->sender(), event->urlList());
}

void AppController::actionSendToBluetooth()
{
    QAction *act = dynamic_cast<QAction *>(sender());
    if (!act)
        return;
    DUrlList urlList = DUrl::fromStringList(act->property("urlList").toStringList());
    fileService->sendToBluetooth(urlList);
}

void AppController::actionAddToBookMark(const QSharedPointer<DFMUrlBaseEvent> &event)
{
    fileService->addToBookmark(event->sender(), event->url());
}

void AppController::actionNewFolder(const QSharedPointer<DFMUrlBaseEvent> &event)
{
    DAbstractFileInfoPointer info = DFileService::instance()->createFileInfo(nullptr, event->url());

    fileService->mkdir(event->sender(), FileUtils::newDocumentUrl(info, tr("New Folder"), QString()));
}

void AppController::actionSelectAll(quint64 winId)
{
    emit fileSignalManager->requestViewSelectAll(static_cast<int>(winId));
}

void AppController::actionClearRecent(const QSharedPointer<DFMMenuActionEvent> &event)
{
    Q_UNUSED(event)
}

void AppController::actionClearRecent()
{
    QFile f(QDir::homePath() + "/.local/share/recently-used.xbel");
    f.open(QIODevice::WriteOnly);
    f.write(empty_recent_file);
    f.close();
}

void AppController::actionClearTrash(const QObject *sender)
{
    DUrlList list;
    list << DUrl::fromTrashFile("/");
    //fix bug 31324,判断当前是否是正在清空回收站，是返回，不是保存状态
    if (DFileService::instance()->getDoClearTrashState()) {
        return;
    }
    DFileService::instance()->setDoClearTrashState(true);

    bool ret = fileService->deleteFiles(sender, list);

    if (ret) {
        DDesktopServices::playSystemSoundEffect(DDesktopServices::SSE_EmptyTrash);
    }
}

void AppController::actionNewWord(const QSharedPointer<DFMUrlBaseEvent> &event)
{
    int windowId = event->windowId();
    bool wayland = DesktopInfo().waylandDectected();
    QString suffix = wayland ? "wps" : "doc";
    DAbstractFileInfoPointer info = DFileService::instance()->createFileInfo(nullptr, event->url());
    FileUtils::cpTemplateFileToTargetDir(info->toLocalFile(), QObject::tr("Document"), suffix, windowId);
}

void AppController::actionNewExcel(const QSharedPointer<DFMUrlBaseEvent> &event)
{
    int windowId = event->windowId();
    bool wayland = DesktopInfo().waylandDectected();
    QString suffix = wayland ? "et" : "xls";
    DAbstractFileInfoPointer info = DFileService::instance()->createFileInfo(nullptr, event->url());
    FileUtils::cpTemplateFileToTargetDir(info->toLocalFile(), QObject::tr("Spreadsheet"), suffix, windowId);
}

void AppController::actionNewPowerpoint(const QSharedPointer<DFMUrlBaseEvent> &event)
{
    int windowId = event->windowId();
    bool wayland = DesktopInfo().waylandDectected();
    QString suffix = wayland ? "dps" : "ppt";
    DAbstractFileInfoPointer info = DFileService::instance()->createFileInfo(nullptr, event->url());
    FileUtils::cpTemplateFileToTargetDir(info->toLocalFile(), QObject::tr("Presentation"), suffix, windowId);
}

void AppController::actionNewText(const QSharedPointer<DFMUrlBaseEvent> &event)
{
    quint64 windowId = event->windowId();
    DAbstractFileInfoPointer info = DFileService::instance()->createFileInfo(nullptr, event->url());
    FileUtils::cpTemplateFileToTargetDir(info->toLocalFile(), QObject::tr("Text"), "txt", windowId);
}

void AppController::actionMount(const QSharedPointer<DFMUrlBaseEvent> &event)
{
    const DUrl &fileUrl = event->url();

    if (fileUrl.scheme() == DFMROOT_SCHEME) {
        auto path = fileUrl.path();
        // mount the stashed remote connections
        if (path.endsWith(SUFFIX_STASHED_REMOTE)) {
            path = RemoteMountsStashManager::normalizeConnUrl(path);
            auto e = dMakeEventPointer<DFMUrlBaseEvent>(event->sender(), path);
            GvfsMountManager::instance()->mount_sync(*e);
            return;
        }

        DAbstractFileInfoPointer fi = fileService->createFileInfo(event->sender(), fileUrl);
        QScopedPointer<DBlockDevice> blk(DDiskManager::createBlockDevice(fi->extraProperties()["udisksblk"].toString()));
        QScopedPointer<DDiskDevice> drv(DDiskManager::createDiskDevice(blk->drive()));

        if (drv && drv->mediaCompatibility().join(" ").contains("optical") && !drv->mediaAvailable() && drv->ejectable()) {
            QtConcurrent::run([](QString drvs) {
                //fix:对于磁盘这块，主要由于光驱不会自动挂载，只有加载成功后鼠标左键双击才会执行此步，现在这样容易导致用户一系列误操作，故关闭。
                //QScopedPointer<DDiskDevice> drv(DDiskManager::createDiskDevice(drvs));
                //drv->eject({});
                Q_UNUSED(drvs)
            }, blk->drive());
            return;
        }
        // 断网时mount Samba无效
        if (blk->device().isEmpty()) {
            dialogManager->showErrorDialog(tr("Mounting device error"), QString());
            qWarning() << "blockDevice is invalid, fileurl is " << fileUrl;
            return;
        }

        DUrl q;
        q.setQuery(blk->device());
        const QSharedPointer<DFMUrlBaseEvent> &e = dMakeEventPointer<DFMUrlBaseEvent>(event->sender(), q);
        actionMount(e);
        return;
    } else if (fileUrl.scheme() == APPENTRY_SCHEME) {
        auto e = dMakeEventPointer<DFMUrlListBaseEvent>(event->sender(), DUrlList() << fileUrl);
        actionOpen(e);
        return;
    }

    QStringList deviceNode = DDiskManager::resolveDeviceNode(fileUrl.query(), {});
    if(deviceNode.isEmpty()) {
        qDebug()<<"============>DeviceNode for "<< fileUrl <<"is empty";
        return;
    }
    QString udiskspath = deviceNode.first();
    QSharedPointer<DBlockDevice> blkdev(DDiskManager::createBlockDevice(udiskspath));
    QSharedPointer<DDiskDevice> drive(DDiskManager::createDiskDevice(blkdev->drive()));
    if (drive->optical()) {
        QtConcurrent::run([ = ] {
            QMutexLocker locker(getOpticalDriveMutex()); //bug 31318 refine

            DISOMasterNS::DeviceProperty dp = ISOMaster->getDevicePropertyCached(fileUrl.query());
            if (dp.devid.length() == 0)
            {
                if (blkdev->mountPoints().size()) {
                    blkdev->unmount({});
                }
                qDebug() << "============>fileUrl.query():" << fileUrl.query();
                if (!ISOMaster->acquireDevice(fileUrl.query())) {
                    ISOMaster->releaseDevice();
                    blkdev->unmount({});
                    QThread::msleep(1000);
                    QScopedPointer<DDiskDevice> diskdev(DDiskManager::createDiskDevice(blkdev->drive()));
                    diskdev->eject({});

                    if (diskdev->optical())
                        QMetaObject::invokeMethod(dialogManager, std::bind(&DialogManager::showErrorDialog, dialogManager, tr("The disc image was corrupted, cannot mount now, please erase the disc first"), QString()), Qt::ConnectionType::QueuedConnection);

                    return;
                }
                dp = ISOMaster->getDeviceProperty();
                ISOMaster->releaseDevice();
            }

            if (!dp.formatted)
            {
                //blkdev->mount({});
                //We have to stick with 'UDisk'Listener until actionOpenDiskInNew*
                //stops relying on doSubscriberAction...
                deviceListener->mount(fileUrl.query());
            }
        });
        return;
    }
    deviceListener->mount(fileUrl.query());
}

void AppController::actionMountImage(const QSharedPointer<DFMUrlBaseEvent> &event)
{
    qDebug() << "mount image:" << event->url();

    DAbstractFileInfoPointer info = DFileService::instance()->createFileInfo(this, event->url());

    QString archiveuri = "";
    if (info && info->canRedirectionFileUrl()) {
        archiveuri = "archive://" + QString(QUrl::toPercentEncoding(info->redirectedFileUrl().toString()));
        qDebug() << "redirect the url to:" << info->redirectedFileUrl();
    } else {
        archiveuri = "archive://" + QString(QUrl::toPercentEncoding(event->url().toString()));
    }

    QStringList args;
    args << "mount" << archiveuri;

    QProcess *gioproc = new QProcess;
    gioproc->start("gio", args);
    connect(gioproc, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, [ = ](int ret) {
        if (ret) {
            dialogManager->showErrorDialog(tr("Mount error: unsupported image format"), QString());
        } else {
            QString double_encoded_uri = QUrl::toPercentEncoding(event->url().toEncoded());
            double_encoded_uri = QUrl::toPercentEncoding(double_encoded_uri);
            QExplicitlySharedDataPointer<DGioFile> f(DGioFile::createFromUri("archive://" + double_encoded_uri));
            if (f->path().length()) {
                this->actionOpen(dMakeEventPointer<DFMUrlListBaseEvent>(event->sender(), DUrlList() << DUrl::fromLocalFile(f->path())));
            }
        }
        gioproc->deleteLater();
    });
}

void AppController::popQueryScanningDialog(QObject *object, std::function<void()> onStop)
{
    QPointer<QObject> pobject = object;
    int code = dialogManager->showStopScanningDialog();
    // 用户选择'终止'
    if (code > 0 && pobject)
        onStop();
}

void AppController::refreshDesktop()
{
    static const QString DesktopService = "com.deepin.dde.desktop";
    static const QString DesktopPath = "/com/deepin/dde/desktop";
    DesktopInterface desktop(DesktopService,
                            DesktopPath,
                            QDBusConnection::sessionBus(),
                            this);
    desktop.asyncCallWithArgumentList(QStringLiteral("Refresh"), QList<QVariant>());
}

void AppController::actionUnmount(const QSharedPointer<DFMUrlBaseEvent> &event)
{
    const DUrl &fileUrl = event->url();
    DAbstractFileInfoPointer fi = fileService->createFileInfo(event->sender(), fileUrl);
    if (fi) {
        const QString &blkStr = fi->extraProperties()["udisksblk"].toString();
        if (!blkStr.isNull() && !blkStr.isEmpty() && m_umountManager && m_umountManager->isScanningBlock(blkStr)) {
            popQueryScanningDialog(this, [this, event, blkStr](){
                if (!m_umountManager) {
                    qWarning() << "m_umountManager is null.";
                    return;
                }
                if (m_umountManager->stopScanBlock(blkStr))
                    doActionUnmount(event);
                else
                    qWarning() << m_umountManager->getErrorMsg();
            });
            // 需要用户确认时弹框，提前结束卸载流程
            return;
        }
    }
    // 其它情况直接走卸载流程
    doActionUnmount(event);
}

void AppController::doActionUnmount(const QSharedPointer<DFMUrlBaseEvent> &event)
{
    const DUrl &fileUrl = event->url();

    if (fileUrl.scheme() == DFMROOT_SCHEME) {
        DAbstractFileInfoPointer fi = fileService->createFileInfo(event->sender(), fileUrl);

        // huawei 50143: 卸载时有任务，提示设备繁忙并且不能中断传输
//        emit fileSignalManager->requestAsynAbortJob(fi->redirectedFileUrl());

        if (fi->suffix() == SUFFIX_UDISKS) {
            //在主线程去调用unmount时如果弹出权限认证窗口，会导致文管界面挂起，
            //在关闭窗口特效情况下，还会出现文管界面绘制不刷新出现重影的情况
            //把unmount相关操作移至子线程
            emit doUnmount(fi->extraProperties()["udisksblk"].toString());
        } else if (fi->suffix() == SUFFIX_GVFSMP) {
            QString path = fi->extraProperties()["rooturi"].toString();
            if (path.isEmpty()) {
                //FIXME(zccrs): 为解决，无法访问smb共享地址时，点击卸载共享目录，创建的fileinfo拿到的path为""，所有就没办法umount，临时解决方案，重Durl中获取
                QString encodePath = QUrl::fromPercentEncoding(fileUrl.path().mid(1).chopped(
                                                                   QString("." SUFFIX_GVFSMP).length()).toUtf8());
                if (encodePath.contains(SMB_SCHEME) && encodePath.split(",").count() >= 2) {
                    path = QString("smb://");
                    QStringList strlist = encodePath.split(",");
                    if (strlist.at(0).contains("="))
                        path = path + strlist.at(0).mid(strlist.at(0).indexOf("=") + 1) + "/";
                    if (strlist.at(1).contains("share=")) {
                        QString shareName = strlist.at(1);
                        path = path + shareName.replace(QString("share="), "") + "/";
                    }
                }
                //fix bug 61610 是ftp和sftp在断网的情况下，卸载ftp或者sftp时rooturi为空，通过url去获取
                if (encodePath.contains(FTP_SCHEME)) {
                    path = encodePath.contains(SFTP_SCHEME) ? QString("sftp://") : QString("ftp://");
                    //匹配ip，找到ip
                    QString Ip = encodePath.indexOf("=") >= 0 ?
                                encodePath.mid(encodePath.indexOf("=") + 1) : QString();
                    path = path + Ip + QString("/");
                }
                qInfo() << "umount path = " << path;
            }

            deviceListener->unmount(path);
        }
    } else if (fileUrl.query().length()) {
        QString dev = fileUrl.query(DUrl::FullyEncoded);
        deviceListener->unmount(dev);
    }
}

void AppController::actionRestore(const QSharedPointer<DFMUrlListBaseEvent> &event)
{
    DFileService::instance()->restoreFile(event->sender(), event->urlList());
}

void AppController::actionRestoreAll(const QSharedPointer<DFMUrlBaseEvent> &event)
{
    DFileService::instance()->restoreFile(event->sender(), DUrlList() << event->url());
}

void AppController::actionEject(const QSharedPointer<DFMUrlBaseEvent> &event)
{
    const DUrl &fileUrl = event->url();

    if (fileUrl.scheme() == DFMROOT_SCHEME) {
        DAbstractFileInfoPointer fi = fileService->createFileInfo(this, fileUrl);

        // Only optical media has eject option, should terminate all oprations while ejecting...
        emit fileSignalManager->requestAsynAbortJob(fi->redirectedFileUrl());
        QtConcurrent::run([fi]() {
            qDebug() << fi->fileUrl().path();
            QString strVolTag = fi->fileUrl().path().remove("/").remove(".localdisk"); // /sr0.localdisk 去头去尾取卷标
            //fix: 刻录期间误操作弹出菜单会引起一系列错误引导，规避用户误操作后引起不必要的错误信息提示
            if (strVolTag.startsWith("sr") && DFMOpticalMediaWidget::g_mapCdStatusInfo[strVolTag].bBurningOrErasing) {
                QMetaObject::invokeMethod(dialogManager, "showErrorDialog", Qt::QueuedConnection,
                                          Q_ARG(QString, tr("The device was not safely removed")),  Q_ARG(QString, tr("Click \"Safely Remove\" and then disconnect it next time")));
            } else {
                bool err = false;
                QScopedPointer<DBlockDevice> blk(DDiskManager::createBlockDevice(fi->extraProperties()["udisksblk"].toString()));
                QScopedPointer<DDiskDevice> drv(DDiskManager::createDiskDevice(blk->drive()));
                QScopedPointer<DBlockDevice> cbblk(DDiskManager::createBlockDevice(blk->cryptoBackingDevice()));
                if (!blk->mountPoints().empty()) {
                    blk->unmount({});
                    QDBusError lastError = blk->lastError();
                    if (lastError.message().contains("target is busy")) {
                        QMetaObject::invokeMethod(dialogManager, "showErrorDialog", Qt::QueuedConnection, Q_ARG(QString, tr("The device was not ejected")),  Q_ARG(QString, tr("Disk is busy, cannot eject now")));
                        return;
                    }
                  
                    if (lastError.type() == QDBusError::Other) { // bug 27164, 取消 应该直接退出操作
                        qDebug() << "blk action has been canceled";
                        return;
                    }

                    if (lastError.type() == QDBusError::NoReply) { // bug 29268, 用户超时操作
                        qDebug() << "action timeout with noreply response";
                        QMetaObject::invokeMethod(dialogManager, "showErrorDialog", Qt::QueuedConnection, Q_ARG(QString, tr("Authentication timed out")),  Q_ARG(QString, ""));
                        //dialogManager->showErrorDialog(tr("Authentication timed out"), QString());
                        return;
                    }
                    err |= blk->lastError().isValid();
                }
                if (blk->cryptoBackingDevice().length() > 1) {
                    err |= cbblk->lastError().isValid();
                    drv.reset(DDiskManager::createDiskDevice(cbblk->drive()));
                }
                drv->eject({});
                err |= drv->lastError().isValid();
                QDBusError dbusError = drv->lastError();
                if (err) {
                    // fix bug #27164 用户在操作其他用户挂载上的设备的时候需要进行提权操作，此时需要输入用户密码，如果用户点击了取消，此时返回 QDBusError::Other
                    // 所以暂时这样处理，处理并不友好。这个 errorType 并不能准确的反馈出用户的操作与错误直接的关系。这里笼统的处理成“设备正忙”也不准确。
                    if (dbusError.isValid() && dbusError.type() != QDBusError::Other) {
                        qDebug() << "disc eject error: " << dbusError.message() << dbusError.name() << dbusError.type();
                        QMetaObject::invokeMethod(dialogManager, "showErrorDialog", Qt::QueuedConnection,
                                                  Q_ARG(QString, tr("The device was not safely removed")), Q_ARG(QString, tr("Click \"Safely Remove\" and then disconnect it next time")));
                    }
                }
            }
        });
    } else {
        deviceListener->eject(fileUrl.query(DUrl::FullyEncoded));
    }
}

void AppController::actionSafelyRemoveDrive(const QSharedPointer<DFMUrlBaseEvent> &event)
{
    const DUrl &fileUrl = event->url();

    QString driveName;
    if (fileUrl.scheme() == DFMROOT_SCHEME) {
        DAbstractFileInfoPointer fi = fileService->createFileInfo(this, fileUrl);
        const QString &blkStr = fi->extraProperties()["udisksblk"].toString();
        if (m_umountManager)
            driveName = m_umountManager->getDriveName(blkStr);
        else
            qWarning() << "m_umountManager is null!";
    }

    if (!driveName.isNull() && !driveName.isEmpty() && m_umountManager && m_umountManager->isScanningDrive(driveName)) {
        popQueryScanningDialog(this, [this, event, driveName](){
            if (!m_umountManager) {
                qWarning() << "m_umountManager is null!";
                return;
            }
            if (m_umountManager->stopScanDrive(driveName))
                doSafelyRemoveDrive(event);
            else
                qWarning() << m_umountManager->getErrorMsg();
        });
        // 需要用户确认时弹框，提前结束卸载流程
        return;
    }
    // 其它情况直接走卸载流程
    doSafelyRemoveDrive(event);
}

void AppController::doSafelyRemoveDrive(const QSharedPointer<DFMUrlBaseEvent> &event)
{
    const DUrl &fileUrl = event->url();

    if (fileUrl.scheme() == DFMROOT_SCHEME) {
        DAbstractFileInfoPointer fi = fileService->createFileInfo(this, fileUrl);

        // bug 29419 期望在外设进行卸载，弹出时，终止复制操作
//        emit fileSignalManager->requestAsynAbortJob(fi->redirectedFileUrl());

        //在主线程去调用unmount时如果弹出权限认证窗口，会导致文管界面挂起，
        //在关闭窗口特效情况下，还会出现文管界面绘制不刷新出现重影的情况
        //把unmount相关操作移至子线程
        emit doSaveRemove(fi->extraProperties()["udisksblk"].toString());
    } else {
        QString unix_device = fileUrl.query(DUrl::FullyEncoded);
        QString drive_unix_device = gvfsMountManager->getDriveUnixDevice(unix_device);
        if (!drive_unix_device.isEmpty()) {
            gvfsMountManager->stop_device(drive_unix_device);
        }
    }
}

void AppController::actionOpenInTerminal(const QSharedPointer<DFMUrlListBaseEvent> &event)
{
    for (const DUrl &url : event->urlList()) {
        fileService->openInTerminal(event->sender(), url);
    }
}

void AppController::actionProperty(const QSharedPointer<DFMUrlListBaseEvent> &event)
{
    DUrlList urlList = event->urlList();

    for (DUrl &url : urlList) {
        DUrl realTargetUrl = url;

        //consider symlink file that links to trash/computer desktop files
        const DAbstractFileInfoPointer &info = fileService->createFileInfo(event->sender(), url);
        if (info && info->isSymLink()) {
            realTargetUrl = info->rootSymLinkTarget();
        }

        if (info && info->scheme() == DFMROOT_SCHEME && info->suffix() == SUFFIX_USRDIR) {
            url = info->redirectedFileUrl();
        }
        // filx bug 60949 当共享文件夹的权限，被服务器改变去掉执行时，计算机界面选中共享文件，
        // 再右键属性，选择属性，是不能显示的，调用checkGvfsMountfileBusy检查一下，弹提示。
        if (url.scheme() == DFMROOT_SCHEME && info->suffix() == SUFFIX_GVFSMP &&
                DFileService::instance()->checkGvfsMountfileBusy(url)) {
            urlList.removeAll(url);
        }

        if (url.isLocalFile()) {
            QString urlSrcPath = url.path();
            QStorageInfo partition(url.path());
            if (partition.isValid() && partition.rootPath() == url.path() && !DDiskManager::resolveDeviceNode(partition.device(), {}).isEmpty()) {
                QString dev(partition.device());
                QString newDev = dev.mid(dev.lastIndexOf('/') + 1);
                url = DUrl(DFMROOT_ROOT + newDev + "." + SUFFIX_UDISKS);
                //fix lvm分区下的逻辑目录打开属性需要对url做转换
                if (dev.contains("/mapper/")) {
                    QList<DAbstractFileInfoPointer> ch = fileService->getChildren(this, DUrl(DFMROOT_ROOT), {}, nullptr);
                    for (auto chi : ch) {
                        if (chi.data()->checkMpsStr(urlSrcPath)) {
                            url = chi->fileUrl();
                            break;
                        }
                    }
                }
            }
        }

        DUrl gvfsmpurl;
        gvfsmpurl.setScheme(DFMROOT_SCHEME);
        gvfsmpurl.setPath("/" + QUrl::toPercentEncoding(url.path()) + "." SUFFIX_GVFSMP);
        DAbstractFileInfoPointer fp(new DFMRootFileInfo(gvfsmpurl));
        if (fp->exists()) {
            url = gvfsmpurl;
        }

        if (info && info->scheme() == BURN_SCHEME && info->fileUrl().burnFilePath().contains(QRegularExpression("^/*$"))) {
            url = DUrl(DFMROOT_ROOT + info->fileUrl().burnDestDevice().mid(QString("/dev/").length()) + "." + SUFFIX_UDISKS);
        }

        if (realTargetUrl.toLocalFile().endsWith(QString(".") + DESKTOP_SURRIX)) {
            DesktopFile df(realTargetUrl.toLocalFile());
            if (df.getDeepinId() == "dde-trash") {
                dialogManager->showTrashPropertyDialog(DFMUrlBaseEvent(event->sender(), realTargetUrl));
                urlList.removeOne(url);
            } else if (df.getDeepinId() == "dde-computer") {
                dialogManager->showComputerPropertyDialog();
                urlList.removeOne(url);
            }
        }
    }

    if (urlList.isEmpty()) {
        return;
    }

    if (urlList.first() == DUrl::fromTrashFile("/")) {
        emit fileSignalManager->requestShowTrashPropertyDialog(DFMUrlBaseEvent(event->sender(), urlList.first()));
    } else if (urlList.first() == DUrl::fromComputerFile("/")) {
        emit fileSignalManager->requestShowComputerPropertyDialog(DFMUrlBaseEvent(event->sender(), urlList.first()));
    } else {
        emit fileSignalManager->requestShowPropertyDialog(DFMUrlListBaseEvent(event->sender(), urlList));
    }
}

void AppController::actionNewWindow(const QSharedPointer<DFMUrlListBaseEvent> &event)
{
    return actionOpenInNewWindow(event);
}

void AppController::actionExit(quint64 winId)
{
    emit fileSignalManager->aboutToCloseLastActivedWindow(static_cast<int>(winId));
}

void AppController::actionSetAsWallpaper(const QSharedPointer<DFMUrlBaseEvent> &event)
{
    const DUrl &fileUrl = event->url();

    if (fileUrl.isLocalFile()) {
        FileUtils::setBackground(fileUrl.toLocalFile());
    } else {
        const DAbstractFileInfoPointer &info = DFileService::instance()->createFileInfo(nullptr, fileUrl);

        if (info) {
            const QString &local_file = info->toLocalFile();

            if (!local_file.isEmpty()) {
                FileUtils::setBackground(local_file);
            }
        }
    }
}

void AppController::actionShare(const QSharedPointer<DFMUrlListBaseEvent> &event)
{
    emit fileSignalManager->requestShowShareOptionsInPropertyDialog(*event.data());
}

void AppController::actionUnShare(const QSharedPointer<DFMUrlBaseEvent> &event)
{
    fileService->unShareFolder(event->sender(), event->url());
}

void AppController::actionConnectToServer(quint64 winId)
{
    dialogManager->showConnectToServerDialog(winId);
}

void AppController::actionSetUserSharePassword(quint64 winId)
{
    dialogManager->showUserSharePasswordSettingDialog(winId);
}

void AppController::actionSettings(quint64 winId)
{
    dialogManager->showGlobalSettingsDialog(winId);
}

void AppController::actionFormatDevice(const QSharedPointer<DFMUrlBaseEvent> &event)
{
    QWidget *w = WindowManager::getWindowById(event->windowId());
    if (!w) {
        return;
    }

    DAbstractFileInfoPointer info = fileService->createFileInfo(nullptr, event->url());
    if (!info) {
        return;
    }

    QSharedPointer<DBlockDevice> blkdev(DDiskManager::createBlockDevice(info->extraProperties()["udisksblk"].toString()));
    QString devicePath = blkdev->device();

    QString cmd = "dde-device-formatter";
    QStringList args;
    args << "-m=" + QString::number(event->windowId()) << devicePath;

    QProcess *process = new QProcess(this);

    connect(process, &QProcess::started, this, [w, process] {
        QWidget *tmpWidget = new QWidget(w);

        tmpWidget->setWindowModality(Qt::WindowModal);
        tmpWidget->setWindowFlags(Qt::Dialog);
        tmpWidget->setAttribute(Qt::WA_DontShowOnScreen);
        tmpWidget->show();

        connect(process, static_cast<void (QProcess::*)(int)>(&QProcess::finished),
                tmpWidget, &QWidget::deleteLater);
        connect(process, static_cast<void (QProcess::*)(QProcess::ProcessError)>(&QProcess::error),
                tmpWidget, &QWidget::deleteLater);
    });

    connect(process, static_cast<void (QProcess::*)(int)>(&QProcess::finished),
            process, &QProcess::deleteLater);
    connect(process, static_cast<void (QProcess::*)(QProcess::ProcessError)>(&QProcess::error),
            process, &QProcess::deleteLater);
    process->startDetached(cmd, args);
}

void AppController::actionOpticalBlank(const QSharedPointer<DFMUrlBaseEvent> &event)
{
    if (DThreadUtil::runInMainThread(dialogManager, &DialogManager::showOpticalBlankConfirmationDialog, DFMUrlBaseEvent(event->sender(), event->url())) == DDialog::Accepted) {
        QtConcurrent::run([ = ] {
            QSharedPointer<FileJob> job(new FileJob(FileJob::OpticalBlank));
            job->moveToThread(qApp->thread());
            job->setWindowId(static_cast<int>(event->windowId()));
            dialogManager->addJob(job);

            DUrl dev(event->url());

            job->doDiscBlank(dev);
            dialogManager->removeJob(job->getJobId());
            //job->deleteLater();
        });
    }
}

void AppController::actionRemoveStashedMount(const QSharedPointer<DFMUrlBaseEvent> &event)
{
    auto path = event->url().path(); // something like "/smb://1.2.3.4/shared-folder.remote"
    path = RemoteMountsStashManager::normalizeConnUrl(path);
    // then find the stashed key of it.
    const auto &&stashedRemoteMounts = RemoteMountsStashManager::remoteMounts();
    QString key;
    for (const auto &mount: stashedRemoteMounts) {
        auto protocol = mount.value(REMOTE_PROTOCOL).toString();
        auto host = mount.value(REMOTE_HOST).toString();
        auto share = mount.value(REMOTE_SHARE).toString();
        if (protocol.isEmpty() || host.isEmpty()) {
            qWarning() << "protocol or host is empty: " << mount;
            continue;
        }

        QString connPath = QString("%1://%2/%3").arg(protocol).arg(host).arg(share);
        if (connPath == path) {
            key = mount.value(REMOTE_KEY).toString();
            break;
        }
    }
    if (!key.isEmpty()) {
        RemoteMountsStashManager::removeRemoteMountItem(key);
    }
}

void AppController::actionctrlL(quint64 winId)
{
    emit fileSignalManager->requestSearchCtrlL(winId);
}

void AppController::actionctrlF(quint64 winId)
{
    emit fileSignalManager->requestSearchCtrlF(winId);
}

void AppController::actionExitCurrentWindow(quint64 winId)
{
    if (WindowManager::getWindowById(winId)) {
        WindowManager::getWindowById(winId)->close();
    }
}

void AppController::actionShowHotkeyHelp(quint64 winId)
{
    if (!WindowManager::getWindowById(winId)) {
        return;
    }
    QRect rect = WindowManager::getWindowById(winId)->geometry();
    QPoint pos(rect.x() + rect.width() / 2, rect.y() + rect.height() / 2);
    Shortcut sc;
    QStringList args;
    QString param1 = "-j=" + sc.toStr();
    QString param2 = "-p=" + QString::number(pos.x()) + "," + QString::number(pos.y());
    args << param1 << param2;
    QProcess::startDetached("deepin-shortcut-viewer", args);
}

void AppController::actionBack(quint64 winId)
{
    DFMEventDispatcher::instance()->processEvent(dMakeEventPointer<DFMBackEvent>(this), qobject_cast<DFileManagerWindow *>(WindowManager::getWindowById(winId)));
}

void AppController::actionForward(quint64 winId)
{
    DFMEventDispatcher::instance()->processEvent(dMakeEventPointer<DFMForwardEvent>(this), qobject_cast<DFileManagerWindow *>(WindowManager::getWindowById(winId)));
}

void AppController::actionForgetPassword(const QSharedPointer<DFMUrlBaseEvent> &event)
{
    QString path;
    DAbstractFileInfoPointer fi = fileService->createFileInfo(event->sender(), event->url());
    if (fi && fi->suffix() == SUFFIX_GVFSMP) {
        path = QUrl(fi->extraProperties()["rooturi"].toString()).toString();
    }
    QJsonObject smbObj = secretManager->getLoginData(path);

    qDebug() << path << smbObj;

    if (!smbObj.empty()) {
        QStringList ids = path.split("/");
        QString domain;
        QString user;
        QString server;
        if (ids.at(2).contains(";")) {
            domain = ids.at(2).split(";").at(0);
            QString userIps = ids.at(2).split(";").at(1);
            if (userIps.contains("@")) {
                user = userIps.split("@").at(0);
                server = userIps.split("@").at(1);
            }
        } else {
            QString userIps = ids.at(2);
            if (userIps.contains("@")) {
                user = userIps.split("@").at(0);
                server = userIps.split("@").at(1);
            } else {
                server = userIps;
            }
        }
        qDebug() <<  smbObj << server;
        QJsonObject obj;
        obj.insert("user", smbObj.value("username").toString());
        obj.insert("domain", smbObj.value("domain").toString());
        obj.insert("protocol", DUrl(smbObj.value("id").toString()).scheme());
        obj.insert("server", server);
        obj.insert("key", smbObj.value("key").toString());
        secretManager->clearPasswordByLoginObj(obj);
    }
    actionUnmount(event);

    auto stashKey = fi->extraProperties()["backer_url"].toString();
    RemoteMountsStashManager::removeRemoteMountItem(stashKey);
}

void AppController::actionOpenFileByApp()
{
    const QAction *action = qobject_cast<QAction *>(sender());

    if (!action) {
        return;
    }

    QString app = action->property("app").toString();
    if (action->property("urls").isValid()) {
        DUrlList fileUrls = qvariant_cast<DUrlList>(action->property("urls"));
        QStringList fileUrlStrs;
        for (const DUrl &url : fileUrls) {
            fileUrlStrs << url.toString();
        }
        FileUtils::openFilesByApp(app, fileUrlStrs);
    } else {
        DUrl fileUrl = qvariant_cast<DUrl>(action->property("url"));
        fileService->openFileByApp(this, app, fileUrl);
    }
}

void AppController::actionSendToRemovableDisk()
{
    const QAction *action = qobject_cast<QAction *>(sender());

    if (!action) {
        return;
    }

    DUrl targetUrl = DUrl(action->property("mounted_root_uri").toString());
    DUrlList urlList = DUrl::fromStringList(action->property("urlList").toStringList());

    // 如果从光驱内发送文件到其他移动设备，将光驱路径转换成本地挂载路径
    for (DUrl &u : urlList) {
        if (u.scheme() == BURN_SCHEME || u.scheme() == TAG_SCHEME || u.scheme() == SEARCH_SCHEME || u.scheme() == RECENT_SCHEME) {
            DAbstractFileInfoPointer fi = fileService->createFileInfo(nullptr, u);
            if (fi)
                u = fi->redirectedFileUrl();
        }
    }

    //fix:修正临时拷贝文件到光盘的路径问题，不是挂载目录，而是临时缓存目录
    QString blkDevice = action->property("blkDevice").toString();

    if (blkDevice.startsWith("sr")) { // fix bug#27909
        QString &strCachePath = DFMOpticalMediaWidget::g_mapCdStatusInfo[blkDevice].cachePath;
        if (strCachePath.isEmpty()) { // fix 未打开文管加载光驱时，结构中cachePath为空，此时往暂存区中发送文件会触发错误流程
            strCachePath = QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation) + "/" + qApp->organizationName() + "/" DISCBURN_STAGING "/_dev_" + blkDevice;
        }
        DUrl tempTargetUrl = DUrl::fromLocalFile(strCachePath);
        fileService->pasteFile(action, DFMGlobal::CopyAction, tempTargetUrl, urlList);
    } else { // other: usb storage and so on
        fileService->pasteFile(action, DFMGlobal::CopyAction, targetUrl, urlList);
    }
}

void AppController::actionStageFileForBurning()
{
    const QAction *action = qobject_cast<QAction *>(sender());

    if (!action) {
        return;
    }

    QString destdev = action->property("dest_drive").toString();
    DUrlList urlList = DUrl::fromStringList(action->property("urlList").toStringList());
    for (DUrl &u : urlList) {
        DAbstractFileInfoPointer fi = fileService->createFileInfo(sender(), u);
        if (fi) { // MasteredMediaFileInfo::canRedirectionFileUrl() 有问题，现在暂时不知道怎么修改
            u = fi->redirectedFileUrl();
        }
    }

    QScopedPointer<DDiskDevice> dev(DDiskManager::createDiskDevice(destdev));
    if (!dev->optical()) {
        QtConcurrent::run([destdev]() {
            QScopedPointer<DDiskDevice> diskDev(DDiskManager::createDiskDevice(destdev));
            diskDev->eject({});
        });
        return;
    }

    DDiskManager diskm;
    for (auto &blks : diskm.blockDevices({})) {
        QScopedPointer<DBlockDevice> blkd(DDiskManager::createBlockDevice(blks));
        if (blkd->drive() == destdev) {
            DUrl dest = DUrl::fromBurnFile(QString(blkd->device()) + "/" BURN_SEG_STAGING "/");
            fileService->pasteFile(action, DFMGlobal::CopyAction, dest, urlList);
            break;
        }
    }
}

QList<QString> AppController::actionGetTagsThroughFiles(const QSharedPointer<DFMGetTagsThroughFilesEvent> &event)
{
    QList<QString> tags{};

    if (static_cast<bool>(event) && (!event->urlList().isEmpty())) {
        tags = DFileService::instance()->getTagsThroughFiles(nullptr, event->urlList());
    }

    return tags;
}

bool AppController::actionRemoveTagsOfFile(const QSharedPointer<DFMRemoveTagsOfFileEvent> &event)
{
    bool value{ false };

    if (event && (event->url().isValid()) && !(event->tags().isEmpty())) {
        QList<QString> tags = event->tags();
        value = DFileService::instance()->removeTagsOfFile(this, event->url(), tags);
    }

    return value;
}

void AppController::actionChangeTagColor(const QSharedPointer<DFMChangeTagColorEvent> &event)
{
    QString tagName = event->m_tagUrl.fileName();
    QString newColor = TagManager::instance()->getColorNameByColor(event->m_newColorForTag);
    TagManager::instance()->changeTagColor(tagName, newColor);
}

void AppController::showTagEdit(const QRect &parentRect, const QPoint &globalPos, const DUrlList &fileList)
{
    DTagEdit *tagEdit = new DTagEdit();

    auto subValue = parentRect.height() - globalPos.y();
    if (subValue < 98) {
        tagEdit->setArrowDirection(DArrowRectangle::ArrowDirection::ArrowBottom);
    }

    tagEdit->setBaseSize(160, 98);
    tagEdit->setFilesForTagging(fileList);
    tagEdit->setAttribute(Qt::WA_DeleteOnClose);

    ///###: Here, Used the position which was stored in DFileView.
    tagEdit->setFocusOutSelfClosing(true);

    QList<QString> sameTagsInDiffFiles{ DFileService::instance()->getTagsThroughFiles(nullptr, fileList) };

    tagEdit->setDefaultCrumbs(sameTagsInDiffFiles);
    tagEdit->show(globalPos.x(), globalPos.y());
}

#ifdef SW_LABEL
void AppController::actionSetLabel(const DFMEvent &event)
{
    if (FileManagerLibrary::instance()->isCompletion()) {
        std::string path = event.fileUrl().toLocalFile().toStdString();
        //        auto_operation(const_cast<char*>(path.c_str()), "020100");
        FileManagerLibrary::instance()->auto_operation()(const_cast<char *>(path.c_str()), "020100");
        qDebug() << "020100" << "set label";
    }
}

void AppController::actionViewLabel(const DFMEvent &event)
{
    if (FileManagerLibrary::instance()->isCompletion()) {
        std::string path = event.fileUrl().toLocalFile().toStdString();
        //        auto_operation(const_cast<char*>(path.c_str()), "010101");
        FileManagerLibrary::instance()->auto_operation()(const_cast<char *>(path.c_str()), "010101");
        qDebug() << "010101" << "view label";
    }
}

void AppController::actionEditLabel(const DFMEvent &event)
{
    if (FileManagerLibrary::instance()->isCompletion()) {
        std::string path = event.fileUrl().toLocalFile().toStdString();
        //        auto_operation(const_cast<char*>(path.c_str()), "010201");
        FileManagerLibrary::instance()->auto_operation()(const_cast<char *>(path.c_str()), "010201");
        qDebug() << "010201" << "edit label";
    }
}

void AppController::actionPrivateFileToPublic(const DFMEvent &event)
{
    if (FileManagerLibrary::instance()->isCompletion()) {
        std::string path = event.fileUrl().toLocalFile().toStdString();
        //        auto_operation(const_cast<char*>(path.c_str()), "010501");
        FileManagerLibrary::instance()->auto_operation()(const_cast<char *>(path.c_str()), "010501");
        qDebug() << "010501" << "private file to public";
    }
}

void AppController::actionByIds(const DFMEvent &event, QString actionId)
{
    if (FileManagerLibrary::instance()->isCompletion()) {
        std::string path = event.fileUrl().toLocalFile().toStdString();
        std::string _actionId = actionId.toStdString();
        FileManagerLibrary::instance()->auto_operation()(const_cast<char *>(path.c_str()), const_cast<char *>(_actionId.c_str()));
        qDebug() << "action by" << actionId;
    }
}

#endif


void AppController::doSubscriberAction(const QString &path)
{
    switch (eventKey()) {
    case Open:
        asyncOpenDisk(path);
        break;
    case OpenNewWindow:
        asyncOpenDiskInNewWindow(path);
        break;
    case OpenNewTab:
        asyncOpenDiskInNewTab(path);
        break;
    default:
        break;
    }
    deviceListener->removeSubscriber(this);
}

QString AppController::createFile(const QString &sourceFile, const QString &targetDir, const QString &baseFileName, WId windowId)
{
    QFileInfo info(sourceFile);

    if (!info.exists()) {
        return QString();
    }

    const QString &targetFile = FileUtils::newDocmentName(targetDir, baseFileName, info.suffix());
    //    AppController::selectionAndRenameFile = qMakePair(DUrl::fromLocalFile(targetFile), windowId);

    if (DFileService::instance()->touchFile(WindowManager::getWindowById(windowId), DUrl::fromLocalFile(targetFile))) {
        QFile target(targetFile);

        if (!target.open(QIODevice::WriteOnly)) {
            return QString();
        }

        QFile source(sourceFile);

        if (!source.open(QIODevice::ReadOnly)) {
            return QString();
        }

        target.write(source.readAll());

        return targetFile;
    }

    return QString();
}

AppController::AppController(QObject *parent) : QObject(parent)
{
    createGVfSManager();
    createUserShareManager();
    createDBusInterface();
    initConnect();
    registerUrlHandle();
}

AppController::~AppController()
{
    m_unmountThread.quit();
    m_unmountThread.wait();
}

void AppController::initConnect()
{
    connect(userShareManager, &UserShareManager::userShareCountChanged,
            fileSignalManager, &FileSignalManager::userShareCountChanged);

    m_unmountWorker = new UnmountWorker;
    m_unmountWorker->moveToThread(&m_unmountThread);
    connect(&m_unmountThread, &QThread::finished, m_unmountWorker, &QObject::deleteLater);
    connect(m_unmountWorker, &UnmountWorker::unmountResult, this, &AppController::showErrorDialog);
    connect(this, &AppController::doUnmount, m_unmountWorker, &UnmountWorker::doUnmount);
    connect(this, &AppController::doSaveRemove, m_unmountWorker, &UnmountWorker::doSaveRemove);
    connect(fileSignalManager, &FileSignalManager::requestFreshAllDesktop,
                this, &AppController::refreshDesktop);

    m_unmountThread.start();

    m_umountManager.reset(new DUMountManager(this));
}

void AppController::createGVfSManager()
{
    networkManager;
    secretManager;
}

void AppController::createUserShareManager()
{
    userShareManager;
}

void AppController::createDBusInterface()
{
    static const QString SessionManagerService = "com.deepin.SessionManager";
    static const QString SessionManagerPath = "/com/deepin/StartManager";

    //创建中不再响应
    if (m_statDBusInterface == CreatingIFS )
        return;
    m_statDBusInterface = CreatingIFS;

    if (!m_startManagerInterface)
        m_startManagerInterface = new StartManagerInterface(SessionManagerService,
                                                        SessionManagerPath,
                                                        QDBusConnection::sessionBus(),
                                                        this);
    if (!m_introspectableInterface) {
        m_introspectableInterface = new IntrospectableInterface(SessionManagerService,
                                                            SessionManagerPath,
                                                            QDBusConnection::sessionBus(),
                                                            this);
        m_introspectableInterface->setTimeout(1000);
    }

    QtConcurrent::run(QThreadPool::globalInstance(),[this]() {
        QDBusPendingReply<QString> reply = m_introspectableInterface->Introspect();
        reply.waitForFinished();
        if (!reply.isFinished() || reply.isError()) {
            qWarning() << "dbus connect error" << QDBusError::errorString(reply.error().type()) << m_introspectableInterface->service();
            m_statDBusInterface = UnkownIFS;
        } else {
            QString xmlCode = reply.argumentAt(0).toString();
            //Introspect会返回空的接口信息，不能作为判断依据，需置为UnkownIFS再次判断
            if (xmlCode.contains("com.deepin.StartManager")) {
                if (xmlCode.contains("LaunchApp")) {
                    m_statDBusInterface = VaildIFS;
                    qInfo() << "com.deepin.SessionManager : StartManager has LaunchApp interface";
                } else {
                    m_statDBusInterface = NoneIFS;
                    qWarning() << "com.deepin.SessionManager : StartManager doesn't have LaunchApp interface.";
                }
            } else {
                qWarning() << "com.deepin.SessionManager Introspect error" << xmlCode;
                m_statDBusInterface = UnkownIFS;
            }

        }
        return;
    });
}

void AppController::showErrorDialog(const QString &title, const QString &content)
{
    dialogManager->showErrorDialog(title, content);
}

bool AppController::checkLaunchAppInterface()
{
    //再次尝试获取接口状态
    qDebug() << "LaunchAppInterface state " << m_statDBusInterface;
    if (m_statDBusInterface == UnkownIFS)
        createDBusInterface();

    //如果正在加载dbus接口，则稍等
    int times = 10;
    while ((m_statDBusInterface == CreatingIFS) && times > 0) {
        qDebug() << "LaunchAppInterface is loading...";
        QThread::msleep(100);
        times--;
    }

    bool ret = m_statDBusInterface == VaildIFS;
    if (!ret)
        qWarning() << "LaunchAppInterface is invaild, state:" << m_statDBusInterface;
    return ret;
}

StartManagerInterface *AppController::startManagerInterface() const
{
    return m_startManagerInterface;
}

void UnmountWorker::doUnmount(const QString &blkStr)
{
    DUMountManager manager;
    if (!manager.umountBlock(blkStr))
        emit unmountResult(tr("The device was not safely unmounted"), manager.getErrorMsg());
}

void UnmountWorker::doSaveRemove(const QString &blkStr)
{
    DUMountManager manager;
    if (!manager.removeDrive(manager.getDriveName(blkStr)))
        emit unmountResult(tr("The device was not safely removed"), manager.getErrorMsg());
}

DesktopInterface::DesktopInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent)
    : QDBusAbstractInterface(service, path, staticInterfaceName(), connection, parent)
{
}

DesktopInterface::~DesktopInterface()
{
}
