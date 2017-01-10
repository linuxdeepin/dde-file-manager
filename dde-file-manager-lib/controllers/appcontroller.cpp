#include "appcontroller.h"
#include "movejobcontroller.h"
#include "trashjobcontroller.h"
#include "copyjobcontroller.h"
#include "deletejobcontroller.h"
#include "filecontroller.h"
#include "recenthistorymanager.h"
#include "trashmanager.h"
#include "searchcontroller.h"
#include "sharecontroler.h"
#include "avfsfilecontroller.h"
#include "bookmarkmanager.h"
#include "networkcontroller.h"
#include "deviceinfo/udisklistener.h"
#include "dfileservices.h"
#include "fileoperations/filejob.h"

#include "app/filesignalmanager.h"
#include "dfmevent.h"
#include "app/define.h"

#include "interfaces/dfmstandardpaths.h"
#include "shutil/fileutils.h"
#include "views/windowmanager.h"
#include "dbusinterface/soundeffect_interface.h"
#include "dbusinterface/commandmanager_interface.h"

#include "gvfs/networkmanager.h"
#include "gvfs/gvfsmountclient.h"
#include "gvfs/secrectmanager.h"
#include "usershare/usersharemanager.h"
#include "dialogs/dialogmanager.h"
#include "widgets/singleton.h"

#include "../deviceinfo/udisklistener.h"

#include <QProcess>
#include <QStorageInfo>
#include <QAction>
#include <DAboutDialog>
#include <qprocess.h>
#include "shutil/shortcut.h"
#include "models/computerdesktopfileinfo.h"
#include "models/trashdesktopfileinfo.h"

DWIDGET_USE_NAMESPACE

QPair<DUrl, int> AppController::selectionAndRenameFile;


class AppControllerPrivate : public AppController {};
Q_GLOBAL_STATIC(AppControllerPrivate, acGlobal);

AppController *AppController::instance()
{
    return acGlobal;
}

void AppController::registerUrlHandle()
{
    DFileService::dRegisterUrlHandler<FileController>(FILE_SCHEME, "");
    DFileService::dRegisterUrlHandler<TrashManager>(TRASH_SCHEME, "");
    DFileService::dRegisterUrlHandler<SearchController>(SEARCH_SCHEME, "");
    DFileService::dRegisterUrlHandler<NetworkController>(NETWORK_SCHEME, "");
    DFileService::dRegisterUrlHandler<NetworkController>(SMB_SCHEME, "");
    DFileService::dRegisterUrlHandler<ShareControler>(USERSHARE_SCHEME, "");
    DFileService::dRegisterUrlHandler<AVFSFileController>(AVFS_SCHEME, "");
}

void AppController::actionOpen(const DFMEvent &event)
{
    const DUrlList& urls = event.fileUrlList();
    if (urls.size() == 1) {
        DFMEvent e = event;

        e << urls.first();
        e << DUrlList();

        fileService->openUrl(e);
    } else {
        foreach (DUrl url, urls) {
            const DAbstractFileInfoPointer info = fileService->createFileInfo(url);
            if (info->isFile()) {
                fileService->openFile(url);
            } else if(info->isDir()) {
                emit fileSignalManager->requestOpenNewWindowByUrl(url, true);
            }
        }
    }
}

void AppController::actionOpenDisk(const DFMEvent &event)
{

    const DUrl& fileUrl = event.fileUrl();
    if (!QStorageInfo(fileUrl.toLocalFile()).isValid()){
        m_fmEvent = event;
        setEventKey(Open);
        actionMount(event);
        deviceListener->addSubscriber(this);
    }else{
        actionOpen(event);
    }

}


void AppController::asycOpenDisk(const QString &path)
{
    m_fmEvent << DUrl(path);
    DUrlList urls;
    urls << DUrl(path);
    m_fmEvent << urls;
    actionOpen(m_fmEvent);
}

void AppController::actionOpenInNewWindow(const DFMEvent &event)
{
    DUrlList urls = event.fileUrlList();;
    foreach (DUrl url, urls) {
        fileService->openNewWindow(url);
    }
}

void AppController::actionOpenInNewTab(const DFMEvent &event)
{
    emit fileSignalManager->requestOpenInNewTab(event);
}

void AppController::actionOpenDiskInNewTab(const DFMEvent &event)
{
    const DUrl& fileUrl = event.fileUrl();
    if (!QStorageInfo(fileUrl.toLocalFile()).isValid()){
        m_fmEvent = event;
        actionMount(event);
        setEventKey(OpenNewTab);
        deviceListener->addSubscriber(this);
    }else{
        //FIXME(zccrs): 为了菜单中能卸载/挂载U盘，url中被设置了query字段，今后应该将此字段移动到继承的FMEvent中
        DFMEvent newEvent = event;
        DUrl newUrl = fileUrl;

        newUrl.setQuery(QString());
        newEvent << newUrl;

        emit fileSignalManager->requestOpenInNewTab(newEvent);
    }
}

void AppController::asycOpenDiskInNewTab(const QString &path)
{
    m_fmEvent << DUrl(path);
    actionOpenDiskInNewTab(m_fmEvent);
}

void AppController::actionOpenDiskInNewWindow(const DFMEvent &event)
{
    const DUrl& fileUrl = event.fileUrl();
    if (!QStorageInfo(fileUrl.toLocalFile()).isValid()){
        m_fmEvent = event;
        actionMount(event);
        setEventKey(OpenNewWindow);
        deviceListener->addSubscriber(this);
    }else{
        //FIXME(zccrs): 为了菜单中能卸载/挂载U盘，url中被设置了query字段，今后应该将此字段移动到继承的FMEvent中
        DFMEvent newEvent = event;
        DUrl newUrl = fileUrl;

        newUrl.setQuery(QString());
        newEvent << (DUrlList() << newUrl);

        actionOpenInNewWindow(newEvent);
    }
}

void AppController::asycOpenDiskInNewWindow(const QString &path)
{
    DUrlList urls;
    urls << DUrl(path);
    m_fmEvent << DUrl(path);
    m_fmEvent << urls;
    actionOpenInNewWindow(m_fmEvent);
}

void AppController::actionOpenAsAdmain(const DFMEvent &event)
{
    QStringList args;
    args << event.fileUrl().toLocalFile();
    qDebug() << args;
    QProcess::startDetached("dde-file-manager-pkexec", args);
}

void AppController::actionOpenWithCustom(const DFMEvent &event)
{
    emit fileSignalManager->requestShowOpenWithDialog(event);
}

void AppController::actionOpenFileLocation(const DFMEvent &event)
{
    const DUrlList& urls = event.fileUrlList();
    foreach (DUrl url, urls) {
        fileService->openFileLocation(url);
    }
}

void AppController::actionCompress(const DFMEvent &event)
{
    const DUrlList& urls = event.fileUrlList();
    fileService->compressFiles(urls);
}

void AppController::actionDecompress(const DFMEvent &event)
{
    const DUrlList &list = event.fileUrlList();
    fileService->decompressFile(list);

}

void AppController::actionDecompressHere(const DFMEvent &event)
{
    const DUrlList &list = event.fileUrlList();
    fileService->decompressFileHere(list);
}

void AppController::actionCut(const DFMEvent &event)
{
    const DUrlList& urls = event.fileUrlList();
    fileService->cutFilesToClipboard(urls);

}

void AppController::actionCopy(const DFMEvent &event)
{
    const DUrlList& urls = event.fileUrlList();
    fileService->copyFilesToClipboard(urls);
}

void AppController::actionPaste(const DFMEvent &event)
{
    fileService->pasteFileByClipboard(event.fileUrl(), event);
}

void AppController::actionRename(const DFMEvent &event)
{
    if(event.parentSource() == DFMEvent::LeftSideBar)
    {
        emit fileSignalManager->requestBookmarkRename(event);
        return;
    }

    emit fileSignalManager->requestRename(event);
}

void AppController::actionRemove(const DFMEvent &event)
{
    const DUrlList& urls = event.fileUrlList();
    const DUrl& fileUrl = event.fileUrl();

    if (event.parentSource() == DFMEvent::LeftSideBar) {
        fileSignalManager->requestBookmarkRemove(event);
    } else if (fileUrl.isRecentFile()) {
        fileSignalManager->requestRecentFileRemove(urls);
    }
}

void AppController::actionDelete(const DFMEvent &event)
{
    fileService->moveToTrash(event);
}

void AppController::actionCompleteDeletion(const DFMEvent &event)
{
    fileService->deleteFiles(event);
}

void AppController::actionCreateSymlink(const DFMEvent &event)
{
//    const DUrl& fileUrl = event.fileUrl();
//    int windowId = event.windowId();
//    FileUtils::createSoftLink(windowId, fileUrl.toLocalFile());
    bool result = fileService->createSymlink(event.fileUrl(), event);
    if(!result){
        dialogManager->showFailToCreateSymlinkDialog();
    }
}

void AppController::actionSendToDesktop(const DFMEvent &event)
{
    fileService->sendToDesktop(event);
}

void AppController::actionAddToBookMark(const DFMEvent &event)
{
    const DUrl& fileUrl = event.fileUrl();
    QString dirName = QDir(fileUrl.toLocalFile()).dirName();
    bookmarkManager->writeIntoBookmark(0, dirName, fileUrl);
    emit fileSignalManager->requestBookmarkAdd(dirName, event);
}

void AppController::actionNewFolder(const DFMEvent &event)
{
    fileService->newFolder(event);
}

void AppController::actionNewFile(const DFMEvent &event)
{
    const DUrl& fileUrl = event.fileUrl();
    fileService->newFile(fileUrl);
}

void AppController::actionSelectAll(const DFMEvent &event)
{
    int windowId = event.windowId();
    emit fileSignalManager->requestViewSelectAll(windowId);
}

void AppController::actionClearRecent(const DFMEvent &event)
{
    Q_UNUSED(event)
    emit fileSignalManager->requestClearRecent();
}

void AppController::actionClearTrash(const DFMEvent &event)
{
    DUrlList list;
    list << DUrl::fromTrashFile("/");

#ifdef DDE_COMPUTER_TRASH
    if(event.fileUrl() == TrashDesktopFileInfo::trashDesktopFileUrl())
        const_cast<DFMEvent&>(event) << DUrl::fromTrashFile("/");
#endif

    const_cast<DFMEvent&>(event) << list;
    bool ret = fileService->deleteFiles(event);
    if (ret){
        SoundEffectInterface* soundEffectInterface = new SoundEffectInterface(SoundEffectInterface::staticServerPath(),
                                                                              SoundEffectInterface::staticInterfacePath(),
                                                                              QDBusConnection::sessionBus(), this);
        soundEffectInterface->PlaySystemSound("trash-empty");
        soundEffectInterface->deleteLater();
    }
}

void AppController::actionNewWord(const DFMEvent &event)
{
    const DUrl& fileUrl = event.fileUrl();
    int windowId = event.windowId();
    QString targetFile = FileUtils::newDocmentName(fileUrl.toLocalFile(), QObject::tr("newDoc"), "doc");
    AppController::selectionAndRenameFile = qMakePair(DUrl::fromLocalFile(targetFile), windowId);
    FileUtils::cpTemplateFileToTargetDir(fileUrl.toLocalFile(), QObject::tr("newDoc"), "doc");
}

void AppController::actionNewExcel(const DFMEvent &event)
{
    const DUrl& fileUrl = event.fileUrl();
    int windowId = event.windowId();
    QString targetFile = FileUtils::newDocmentName(fileUrl.toLocalFile(), QObject::tr("newExcel"), "xls");
    AppController::selectionAndRenameFile = qMakePair(DUrl::fromLocalFile(targetFile), windowId);
    FileUtils::cpTemplateFileToTargetDir(fileUrl.toLocalFile(), QObject::tr("newExcel"), "xls");
}

void AppController::actionNewPowerpoint(const DFMEvent &event)
{
    const DUrl& fileUrl = event.fileUrl();
    int windowId = event.windowId();
    QString targetFile = FileUtils::newDocmentName(fileUrl.toLocalFile(), QObject::tr("newPowerPoint"), "ppt");
    AppController::selectionAndRenameFile = qMakePair(DUrl::fromLocalFile(targetFile), windowId);
    FileUtils::cpTemplateFileToTargetDir(fileUrl.toLocalFile(), QObject::tr("newPowerPoint"), "ppt");
}

void AppController::actionNewText(const DFMEvent &event)
{
    const DUrl& fileUrl = event.fileUrl();
    int windowId = event.windowId();
    QString targetFile = FileUtils::newDocmentName(fileUrl.toLocalFile(), QObject::tr("newText"), "txt");
    AppController::selectionAndRenameFile = qMakePair(DUrl::fromLocalFile(targetFile), windowId);
    FileUtils::cpTemplateFileToTargetDir(fileUrl.toLocalFile(), QObject::tr("newText"), "txt");
}

void AppController::actionMount(const DFMEvent &event)
{
    const DUrl& fileUrl = event.fileUrl();
    deviceListener->mount(fileUrl.query());
}

void AppController::actionUnmount(const DFMEvent &event)
{
    const DUrl& fileUrl = event.fileUrl();
    deviceListener->unmount(fileUrl.query(DUrl::FullyEncoded));
}

void AppController::actionRestore(const DFMEvent &event)
{
    DUrlList urls;

    for (const DUrl &url : event.fileUrlList()) {
        if (url.isSearchFile())
            urls << url.searchedFileUrl();
        else
            urls << url;
    }

    TrashManager::restoreTrashFile(urls, event);
}

void AppController::actionRestoreAll(const DFMEvent &event)
{
    TrashManager::restoreAllTrashFile(event);
}

void AppController::actionEject(const DFMEvent &event)
{
    const DUrl& fileUrl = event.fileUrl();
    deviceListener->eject(fileUrl.query(DUrl::FullyEncoded));
}

void AppController::actionOpenInTerminal(const DFMEvent &event)
{
    if (event.fileUrlList().isEmpty()) {
        fileService->openInTerminal(event.fileUrl());

        return;
    }

    for (const DUrl &url: event.fileUrlList()) {
        fileService->openInTerminal(url);
    }
}

void AppController::actionProperty(const DFMEvent &event)
{
    if (event.fileUrlList().isEmpty())
        return;

#ifdef DDE_COMPUTER_TRASH
    if(event.fileUrlList().count() == 1){
        if(event.fileUrlList().first() == TrashDesktopFileInfo::trashDesktopFileUrl()){
            const_cast<DFMEvent&>(event) << DUrl::fromTrashFile("/");
            emit fileSignalManager->requestShowTrashPropertyDialog(event);
            return;
        }
    } else{
        DUrlList urlList = event.fileUrlList();
        if(urlList.contains(TrashDesktopFileInfo::trashDesktopFileUrl())){
            const_cast<DFMEvent&>(event) << DUrl::fromTrashFile("/");
            emit fileSignalManager->requestShowTrashPropertyDialog(event);
            urlList.removeOne(TrashDesktopFileInfo::trashDesktopFileUrl());
        }

        if(urlList.contains(ComputerDesktopFileInfo::computerDesktopFileUrl()))
            urlList.removeOne(ComputerDesktopFileInfo::computerDesktopFileUrl());

        if(urlList.isEmpty())
            return;

        const_cast<DFMEvent&>(event) << urlList;
    }
#endif

    if (event.fileUrlList().first() == DUrl::fromTrashFile("/")){
        emit fileSignalManager->requestShowTrashPropertyDialog(event);
    }else{
        emit fileSignalManager->requestShowPropertyDialog(event);
    }
}

void AppController::actionNewWindow(const DFMEvent &event)
{
    bool open_ok = false;

    for (const DUrl &fileUrl : event.fileUrlList()) {
        const DAbstractFileInfoPointer &fileInfo = fileService->createFileInfo(fileUrl);

        if (fileInfo->isDir()) {
            fileService->openNewWindow(fileUrl);
            open_ok = true;
        }
    }

    if (!open_ok) {
        fileService->openNewWindow(event.fileUrl());

        return;
    }
}

void AppController::actionHelp(const DFMEvent &event)
{
    Q_UNUSED(event)
    QStringList args;
    args << qApp->applicationName();
    QProcess::startDetached("dman", args);
}

void AppController::actionAbout(const DFMEvent &event)
{
    Q_UNUSED(event)
    emit fileSignalManager->showAboutDialog(event);
}

void AppController::actionExit(const DFMEvent &event)
{
    int windowId = event.windowId();
    emit fileSignalManager->aboutToCloseLastActivedWindow(windowId);
}

void AppController::actionSetAsWallpaper(const DFMEvent &event)
{
    const DUrl& fileUrl = event.fileUrl();
    FileUtils::setBackground(fileUrl.toLocalFile());
}

void AppController::actionShare(const DFMEvent &event)
{
    emit fileSignalManager->requestShowShareOptionsInPropertyDialog(event);
}

void AppController::actionUnShare(const DFMEvent &event)
{
    fileService->unShareFolder(event.fileUrl());
}

void AppController::actionSetUserSharePassword(const DFMEvent &event)
{
    dialogManager->showUserSharePasswordSettingDialog(event);
}

void AppController::actionFormatDevice(const DFMEvent &event)
{

    UDiskDeviceInfoPointer info = deviceListener->getDeviceByDeviceID(event.fileUrl().query());
    QString devicePath = info->getPath();

    QString cmd = "usb-device-formatter-pkexec";
    QStringList args;
    args << devicePath;
    QProcess::startDetached(cmd,args);
}

void AppController::actionctrlL(const DFMEvent &event)
{
    emit fileSignalManager->requestSearchCtrlL(event);
}

void AppController::actionctrlF(const DFMEvent &event)
{
    emit fileSignalManager->requestSearchCtrlF(event);
}

void AppController::actionExitCurrentWindow(const DFMEvent &event)
{
    int windowId = event.windowId();
    WindowManager::getWindowById(windowId)->close();
}

void AppController::actionShowHotkeyHelp(const DFMEvent &event)
{
    Q_UNUSED(event)
    QRect rect=WindowManager::getWindowById(event.windowId())->geometry();
    QPoint pos(rect.x() + rect.width()/2 , rect.y() + rect.height()/2);
    Shortcut sc;
    QStringList args;
    QString param1 = "-j="+sc.toStr();
    QString param2 = "-p=" + QString::number(pos.x()) + "," + QString::number(pos.y());
    args<<param1<<param2;
    QProcess::startDetached("deepin-shortcut-viewer",args);
}

void AppController::actionBack(const DFMEvent &event)
{
    emit fileSignalManager->requestBack(event);
}

void AppController::actionForward(const DFMEvent &event)
{
    emit fileSignalManager->requestForward(event);
}

void AppController::actionForgetPassword(const DFMEvent &event)
{
    QString path = event.fileUrl().query();

    QJsonObject smbObj = secrectManager->getLoginData(path);
    if (smbObj.empty()){
        if (path.endsWith("/")){
            path = path.left(path.length() - 1);
            smbObj = secrectManager->getLoginData(path);

            if (smbObj.isEmpty()){
                QString share = path.split("/").last();
                path = path.left(path.length() - share.length());
                path += share.toUpper();
                smbObj = secrectManager->getLoginData(path);
            }
        }else{
            path += "/";
            smbObj = secrectManager->getLoginData(path);
            if (smbObj.isEmpty()){
                QStringList _pl = path.split("/");
                QString share = _pl.at(_pl.length() - 2);
                path = path.left(path.length() - share.length() - 1);
                path += share.toUpper();
                path += "/";
                smbObj = secrectManager->getLoginData(path);
            }
        }
    }

    qDebug() << path << smbObj;

    if (!smbObj.empty()){
        QStringList ids = path.split("/");
        QString domain;
        QString user;
        QString server;
        if (ids.at(2).contains(";")){
            domain = ids.at(2).split(";").at(0);
            QString userIps = ids.at(2).split(";").at(1);
            if (userIps.contains("@")){
                user = userIps.split("@").at(0);
                server = userIps.split("@").at(1);
            }
        }else{
            QString userIps = ids.at(2);
            if (userIps.contains("@")){
                user = userIps.split("@").at(0);
                server = userIps.split("@").at(1);
            }else{
                server = userIps;
            }
        }
        qDebug() <<  smbObj << server;
        QJsonObject obj;
        obj.insert("user", smbObj.value("username").toString());
        obj.insert("domain", smbObj.value("domain").toString());
        obj.insert("protocol", "smb");
        obj.insert("server", server);
        secrectManager->clearPasswordByLoginObj(obj);
    }
    actionUnmount(event);
}

void AppController::actionOpenFileByApp()
{
    QAction* dAction = qobject_cast<QAction*>(sender());
    QString app = dAction->property("app").toString();
    DUrl fileUrl(dAction->property("url").toUrl());
    fileService->openFileByApp(fileUrl, app);
}

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

AppController::AppController(QObject *parent) : QObject(parent)
{
    createGVfSManager();
    createUserShareManager();
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
    secrectManager;
}

void AppController::createUserShareManager()
{
    userShareManager;
}
