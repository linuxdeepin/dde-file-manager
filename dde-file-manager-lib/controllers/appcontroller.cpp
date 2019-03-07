/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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

#include "appcontroller.h"
#include "movejobcontroller.h"
#include "trashjobcontroller.h"
#include "copyjobcontroller.h"
#include "deletejobcontroller.h"
#include "filecontroller.h"
#include "trashmanager.h"
#include "searchcontroller.h"
#include "sharecontroler.h"
#include "avfsfilecontroller.h"
#include "mountcontroller.h"
#include "bookmarkmanager.h"
#include "networkcontroller.h"
#include "arrangeddesktopcontroller.h"
#include "deviceinfo/udisklistener.h"
#include "dfileservices.h"
#include "fileoperations/filejob.h"
#include "dfmeventdispatcher.h"
#include "dfmapplication.h"

#include "app/filesignalmanager.h"
#include "dfmevent.h"
#include "app/define.h"

#include "interfaces/dfmstandardpaths.h"
#include "shutil/fileutils.h"
#include "views/windowmanager.h"
#include "views/dfilemanagerwindow.h"
#include "views/dtagedit.h"
#include "dbusinterface/commandmanager_interface.h"

#include "gvfs/networkmanager.h"
#include "gvfs/gvfsmountclient.h"
#include "gvfs/gvfsmountmanager.h"
#include "gvfs/secretmanager.h"
#include "usershare/usersharemanager.h"
#include "dialogs/dialogmanager.h"
#include "singleton.h"

#include "tagcontroller.h"
#include "recentcontroller.h"
#include "views/drenamebar.h"
#include "shutil/filebatchprocess.h"

#include "../deviceinfo/udisklistener.h"

#include <QProcess>
#include <QStorageInfo>
#include <QAction>
#include <DAboutDialog>
#include <qprocess.h>
#include <QtConcurrent>

#include <DApplication>
#include <DDesktopServices>

#include "tag/tagutil.h"
#include "tag/tagmanager.h"
#include "shutil/shortcut.h"
//#include "views/dbookmarkitem.h"
#include "models/desktopfileinfo.h"
#include "controllers/tagmanagerdaemoncontroller.h"

#ifdef SW_LABEL
#include "sw_label/filemanagerlibrary.h"
#endif

using namespace Dtk::Widget;

///###: be used for tag protocol.
template<typename Ty>
using iterator = typename QList<Ty>::iterator;

template<typename Ty>
using citerator = typename QList<Ty>::const_iterator;


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
    DFileService::setFileUrlHandler(TRASH_SCHEME, "", new TrashManager());
    DFileService::dRegisterUrlHandler<SearchController>(SEARCH_SCHEME, "");
    DFileService::dRegisterUrlHandler<NetworkController>(NETWORK_SCHEME, "");
    DFileService::dRegisterUrlHandler<NetworkController>(SMB_SCHEME, "");
    DFileService::dRegisterUrlHandler<NetworkController>(SFTP_SCHEME, "");
    DFileService::dRegisterUrlHandler<NetworkController>(FTP_SCHEME, "");
    DFileService::dRegisterUrlHandler<NetworkController>(DAV_SCHEME, "");
    DFileService::dRegisterUrlHandler<ShareControler>(USERSHARE_SCHEME, "");
    DFileService::dRegisterUrlHandler<AVFSFileController>(AVFS_SCHEME, "");
    DFileService::dRegisterUrlHandler<MountController>(MOUNT_SCHEME, "");

    DFileService::dRegisterUrlHandler<TagController>(TAG_SCHEME, "");
    DFileService::dRegisterUrlHandler<RecentController>(RECENT_SCHEME, "");
#ifdef QT_DEBUG
    DFileService::dRegisterUrlHandler<ArrangedDesktopController>("dfmad", "");
#endif // QT_DEBUG
}

void AppController::actionOpen(const QSharedPointer<DFMUrlListBaseEvent> &event)
{
    const DUrlList &urls = event->urlList();

    if (urls.isEmpty()) {
        return;
    }

    if (urls.size() > 1 || DFMApplication::instance()->appAttribute(DFMApplication::AA_AllwayOpenOnNewWindow).toBool()) {
        DFMEventDispatcher::instance()->processEvent<DFMOpenUrlEvent>(event->sender(), urls, DFMOpenUrlEvent::ForceOpenNewWindow);
    } else {
        DFMEventDispatcher::instance()->processEventAsync<DFMOpenUrlEvent>(event->sender(), urls, DFMOpenUrlEvent::OpenInCurrentWindow);
    }
}

void AppController::actionOpenDisk(const QSharedPointer<DFMUrlBaseEvent> &event)
{
    const DUrl &fileUrl = event->url();
    QString id = fileUrl.query();

    if (!id.isEmpty()) {
        const QDiskInfo &diskInfo = gvfsMountManager->getDiskInfo(id);
        if (diskInfo.can_mount()) {
            m_fmEvent = event;
            setEventKey(Open);
            actionMount(event);
            deviceListener->addSubscriber(this);
        }
        if (diskInfo.can_unmount()) {
            const QSharedPointer<DFMUrlListBaseEvent> &e = dMakeEventPointer<DFMUrlListBaseEvent>(event->sender(), DUrlList() << event->url());
            e->setWindowId(event->windowId());

            actionOpen(e);
        }
    }
}


void AppController::asycOpenDisk(const QString &path)
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
    if (!QStorageInfo(fileUrl.toLocalFile()).isValid()) {
        m_fmEvent = event;
        actionMount(event);
        setEventKey(OpenNewTab);
        deviceListener->addSubscriber(this);
    } else {
        //FIXME(zccrs): 为了菜单中能卸载/挂载U盘，url中被设置了query字段，今后应该将此字段移动到继承的FMEvent中
        DUrl newUrl = fileUrl;

        newUrl.setQuery(QString());

        actionOpenInNewTab(dMakeEventPointer<DFMUrlBaseEvent>(event->sender(), newUrl));
    }
}

void AppController::asycOpenDiskInNewTab(const QString &path)
{
    m_fmEvent->setData(DUrl(path));
    actionOpenDiskInNewTab(m_fmEvent.staticCast<DFMUrlBaseEvent>());
}

void AppController::actionOpenDiskInNewWindow(const QSharedPointer<DFMUrlBaseEvent> &event)
{
    const DUrl &fileUrl = event->url();
    if (!QStorageInfo(fileUrl.toLocalFile()).isValid()) {
        m_fmEvent = event;
        actionMount(event);
        setEventKey(OpenNewWindow);
        deviceListener->addSubscriber(this);
    } else {
        //FIXME(zccrs): 为了菜单中能卸载/挂载U盘，url中被设置了query字段，今后应该将此字段移动到继承的FMEvent中
        DUrl newUrl = fileUrl;

        newUrl.setQuery(QString());
        const QSharedPointer<DFMUrlListBaseEvent> newEvent(new DFMUrlListBaseEvent(event->sender(), DUrlList() << newUrl));

        newEvent->setWindowId(event->windowId());

        actionOpenInNewWindow(newEvent);
    }
}

void AppController::asycOpenDiskInNewWindow(const QString &path)
{
    DUrlList urls;
    urls << DUrl(path);
    m_fmEvent->setData(urls);
    actionOpenInNewWindow(m_fmEvent.staticCast<DFMUrlListBaseEvent>());
}

void AppController::actionOpenAsAdmin(const QSharedPointer<DFMUrlBaseEvent> &event)
{
    QStringList args;
    args << event->url().toString();
    qDebug() << args;
    QProcess::startDetached("dde-file-manager-pkexec", args);
}

void AppController::actionOpenWithCustom(const QSharedPointer<DFMUrlBaseEvent> &event)
{
    emit fileSignalManager->requestShowOpenWithDialog(DFMUrlBaseEvent(event->sender(), event->url()));
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
    fileService->pasteFileByClipboard(event->sender(), event->url());
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
    fileService->moveToTrash(event->sender(), event->urlList());
}

void AppController::actionCompleteDeletion(const QSharedPointer<DFMUrlListBaseEvent> &event)
{
    fileService->deleteFiles(event->sender(), event->urlList());
}

void AppController::actionCreateSymlink(const QSharedPointer<DFMUrlBaseEvent> &event)
{
    fileService->createSymlink(event->sender(), event->url());
}

void AppController::actionSendToDesktop(const QSharedPointer<DFMUrlListBaseEvent> &event)
{
    fileService->sendToDesktop(event->sender(), event->urlList());
}

void AppController::actionAddToBookMark(const QSharedPointer<DFMUrlBaseEvent> &event)
{
    fileService->addToBookmark(event->sender(), event->url());
}

void AppController::actionNewFolder(const QSharedPointer<DFMUrlBaseEvent> &event)
{
    const QString targetDir = event->url().toLocalFile();
    const QString name = FileUtils::newDocmentName(targetDir, tr("New Folder"), QString());

    fileService->mkdir(event->sender(), DUrl::fromLocalFile(name));
}

void AppController::actionSelectAll(quint64 winId)
{
    emit fileSignalManager->requestViewSelectAll(winId);
}

void AppController::actionClearRecent(const QSharedPointer<DFMMenuActionEvent> &event)
{
    Q_UNUSED(event)
}

void AppController::actionClearTrash(const QObject *sender)
{
    DUrlList list;
    list << DUrl::fromTrashFile("/");

    bool ret = fileService->deleteFiles(sender, list);

    if (ret) {
        DDesktopServices::playSystemSoundEffect(DDesktopServices::SSE_EmptyTrash);
    }
}

void AppController::actionNewWord(const QSharedPointer<DFMUrlBaseEvent> &event)
{
    const DUrl &fileUrl = event->url();
    int windowId = event->windowId();
//    QString targetFile = FileUtils::newDocmentName(fileUrl.toLocalFile(), QObject::tr("Document"), "doc");
//    AppController::selectionAndRenameFile = qMakePair(DUrl::fromLocalFile(targetFile), windowId);
    FileUtils::cpTemplateFileToTargetDir(fileUrl.toLocalFile(), QObject::tr("Document"), "doc", windowId);
}

void AppController::actionNewExcel(const QSharedPointer<DFMUrlBaseEvent> &event)
{
    const DUrl &fileUrl = event->url();
    int windowId = event->windowId();
//    QString targetFile = FileUtils::newDocmentName(fileUrl.toLocalFile(), QObject::tr("Spreadsheet"), "xls");
//    AppController::selectionAndRenameFile = qMakePair(DUrl::fromLocalFile(targetFile), windowId);
    FileUtils::cpTemplateFileToTargetDir(fileUrl.toLocalFile(), QObject::tr("Spreadsheet"), "xls", windowId);
}

void AppController::actionNewPowerpoint(const QSharedPointer<DFMUrlBaseEvent> &event)
{
    const DUrl &fileUrl = event->url();
    int windowId = event->windowId();
//    QString targetFile = FileUtils::newDocmentName(fileUrl.toLocalFile(), QObject::tr("Presentation"), "ppt");
//    AppController::selectionAndRenameFile = qMakePair(DUrl::fromLocalFile(targetFile), windowId);
    FileUtils::cpTemplateFileToTargetDir(fileUrl.toLocalFile(), QObject::tr("Presentation"), "ppt", windowId);
}

void AppController::actionNewText(const QSharedPointer<DFMUrlBaseEvent> &event)
{
    const DUrl &fileUrl = event->url();
    int windowId = event->windowId();
//    QString targetFile = FileUtils::newDocmentName(fileUrl.toLocalFile(), QObject::tr("Text"), "txt");
//    AppController::selectionAndRenameFile = qMakePair(DUrl::fromLocalFile(targetFile), windowId);
    FileUtils::cpTemplateFileToTargetDir(fileUrl.toLocalFile(), QObject::tr("Text"), "txt", windowId);
}

void AppController::actionMount(const QSharedPointer<DFMUrlBaseEvent> &event)
{
    const DUrl &fileUrl = event->url();
    deviceListener->mount(fileUrl.query());
}

void AppController::actionUnmount(const QSharedPointer<DFMUrlBaseEvent> &event)
{
    const DUrl &fileUrl = event->url();
    deviceListener->unmount(fileUrl.query(DUrl::FullyEncoded));
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
    deviceListener->eject(fileUrl.query(DUrl::FullyEncoded));
}

void AppController::actionSafelyRemoveDrive(const QSharedPointer<DFMUrlBaseEvent> &event)
{
    const DUrl &fileUrl = event->url();
    QString unix_device = fileUrl.query(DUrl::FullyEncoded);
    QString drive_unix_device = gvfsMountManager->getDriveUnixDevice(unix_device);
    if (!drive_unix_device.isEmpty()) {
        gvfsMountManager->stop_device(drive_unix_device);
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

    foreach (const DUrl &url, urlList) {
        DUrl realTargetUrl = url;

        //consider symlink file that links to trash/computer desktop files
        const DAbstractFileInfoPointer &info = fileService->createFileInfo(event->sender(), url);
        if (info && info->isSymLink()) {
            realTargetUrl = info->rootSymLinkTarget();
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

void AppController::actionHelp()
{
    class PublicApplication : public DApplication {
        public: using  DApplication::handleHelpAction;
    };
    reinterpret_cast<PublicApplication*>(DApplication::instance())->handleHelpAction();
}

void AppController::actionAbout(quint64 winId)
{
    emit fileSignalManager->showAboutDialog(winId);
}

void AppController::actionExit(quint64 winId)
{
    emit fileSignalManager->aboutToCloseLastActivedWindow(winId);
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

    UDiskDeviceInfoPointer info = deviceListener->getDeviceByDeviceID(event->url().query());
    if (!info) {
        return;
    }

    QString devicePath = info->getPath();

    QString cmd = "usb-device-formatter-pkexec";
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
    WindowManager::getWindowById(winId)->close();
}

void AppController::actionShowHotkeyHelp(quint64 winId)
{
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
    QString path = event->url().query();

    QJsonObject smbObj = secretManager->getLoginData(path);
    if (smbObj.empty()) {
        if (path.endsWith("/")) {
            path = path.left(path.length() - 1);
            smbObj = secretManager->getLoginData(path);

            if (smbObj.isEmpty()) {
                QString share = path.split("/").last();
                path = path.left(path.length() - share.length());
                path += share.toUpper();
                smbObj = secretManager->getLoginData(path);
            }
        } else {
            path += "/";
            smbObj = secretManager->getLoginData(path);
            if (smbObj.isEmpty()) {
                QStringList _pl = path.split("/");
                QString share = _pl.at(_pl.length() - 2);
                path = path.left(path.length() - share.length() - 1);
                path += share.toUpper();
                path += "/";
                smbObj = secretManager->getLoginData(path);
            }
        }
    }

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
        secretManager->clearPasswordByLoginObj(obj);
    }
    actionUnmount(event);
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
        for (const DUrl& url : fileUrls) {
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

    fileService->pasteFile(action, DFMGlobal::CopyAction, targetUrl, urlList);
}

QList<QString> AppController::actionGetTagsThroughFiles(const QSharedPointer<DFMGetTagsThroughFilesEvent> &event)
{
    QList<QString> tags{};

    if(static_cast<bool>(event) && (!event->urlList().isEmpty())){
        tags = DFileService::instance()->getTagsThroughFiles(nullptr, event->urlList());
    }

    return tags;
}

bool AppController::actionRemoveTagsOfFile(const QSharedPointer<DFMRemoveTagsOfFileEvent>& event)
{
    bool value{ false };

    if(event && (event->url().isValid()) && !(event->tags().isEmpty())){
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

void AppController::showTagEdit(const QPoint &globalPos, const DUrlList &fileList)
{
    DTagEdit *tagEdit = new DTagEdit();

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
        asycOpenDisk(path);
        break;
    case OpenNewWindow:
        asycOpenDiskInNewWindow(path);
        break;
    case OpenNewTab:
        asycOpenDiskInNewTab(path);
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

void AppController::initConnect()
{
    connect(userShareManager, &UserShareManager::userShareCountChanged,
            fileSignalManager, &FileSignalManager::userShareCountChanged);
}

void AppController::createGVfSManager()
{
    networkManager;
    gvfsMountClient;
    secretManager;
}

void AppController::createUserShareManager()
{
    userShareManager;
}

void AppController::createDBusInterface()
{
    m_startManagerInterface = new StartManagerInterface("com.deepin.SessionManager",
            "/com/deepin/StartManager",
            QDBusConnection::sessionBus(),
            this);
    m_introspectableInterface = new IntrospectableInterface("com.deepin.SessionManager",
            "/com/deepin/StartManager",
            QDBusConnection::sessionBus(),
            this);

    QtConcurrent::run(QThreadPool::globalInstance(), [&] {
        QDBusPendingReply<QString> reply = m_introspectableInterface->Introspect();
        reply.waitForFinished();
        if (reply.isFinished())
        {
            QString xmlCode = reply.argumentAt(0).toString();
            if (xmlCode.contains("LaunchApp")) {
                qDebug() << "com.deepin.SessionManager : StartManager has LaunchApp interface";
                setHasLaunchAppInterface(true);
            } else {
                qDebug() << "com.deepin.SessionManager : StartManager doesn't have LaunchApp interface";
            }
        }
    });
}

void AppController::setHasLaunchAppInterface(bool hasLaunchAppInterface)
{
    m_hasLaunchAppInterface = hasLaunchAppInterface;
}

bool AppController::hasLaunchAppInterface() const
{
    return m_hasLaunchAppInterface;
}

StartManagerInterface *AppController::startManagerInterface() const
{
    return m_startManagerInterface;
}
