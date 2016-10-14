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
#include "bookmarkmanager.h"
#include "networkcontroller.h"
#include "deviceinfo/udisklistener.h"
#include "fileservices.h"
#include "fileoperations/filejob.h"

#include "app/filesignalmanager.h"
#include "app/fmevent.h"
#include "app/global.h"

#include "interfaces/dfmstandardpaths.h"
#include "shutil/fileutils.h"
#include "views/windowmanager.h"
#include "dbusinterface/soundeffect_interface.h"

#include "gvfs/networkmanager.h"
#include "gvfs/gvfsmountclient.h"
#include "gvfs/secrectmanager.h"
#include "usershare/usersharemanager.h"
#include "widgets/singleton.h"

#include "../deviceinfo/udisklistener.h"

#include <QProcess>
#include <QStorageInfo>
#include <DAboutDialog>
#include <qprocess.h>
#include "shutil/shortcut.h"

QPair<DUrl, int> AppController::selectionAndRenameFile;

AppController::AppController(QObject *parent) : QObject(parent)
{
    FileServices::instance()->setFileUrlHandler(RECENT_SCHEME, "", new RecentHistoryManager(this));
    FileServices::dRegisterUrlHandler<FileController>(FILE_SCHEME, "");
    FileServices::dRegisterUrlHandler<TrashManager>(TRASH_SCHEME, "");
    FileServices::dRegisterUrlHandler<SearchController>(SEARCH_SCHEME, "");
    FileServices::dRegisterUrlHandler<NetworkController>(NETWORK_SCHEME, "");
    FileServices::dRegisterUrlHandler<NetworkController>(SMB_SCHEME, "");
    FileServices::dRegisterUrlHandler<ShareControler>(USERSHARE_SCHEME, "");
    createGVfSManager();
    createUserShareManager();
    initConnect();
}

void AppController::initConnect()
{
    connect(userShareManager, &UserShareManager::userShareChanged,
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

void AppController::actionOpen(const FMEvent &event)
{
    const DUrlList& urls = event.fileUrlList();

    if (urls.size() == 1) {
        const_cast<FMEvent&>(event) << urls.first();

        fileService->openUrl(event);
    } else {
        foreach (DUrl url, urls) {
            if (url.isRecentFile()) {
                url.setScheme(FILE_SCHEME);
            }

            if (url.isLocalFile()) {
                QFileInfo info(url.toLocalFile());

                if (info.isFile()) {
                    fileService->openFile(url);
                } else if(info.isDir()) {
                    emit fileSignalManager->requestOpenNewWindowByUrl(url, true);
                }
            } else if (url.isSMBFile()) {
                emit fileSignalManager->requestOpenNewWindowByUrl(url, true);
            }
        }
    }
}

void AppController::actionOpenDisk(const FMEvent &event)
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

void AppController::actionOpenInNewWindow(const FMEvent &event)
{
    const DUrl& fileUrl = event.fileUrl();
    fileService->openNewWindow(fileUrl);
}

void AppController::actionOpenInNewTab(const FMEvent &event)
{
    qDebug() << event;
    emit fileSignalManager->requestOpenInNewTab(event);
}

void AppController::actionOpenDiskInNewWindow(const FMEvent &event)
{
    const DUrl& fileUrl = event.fileUrl();
    if (!QStorageInfo(fileUrl.toLocalFile()).isValid()){
        m_fmEvent = event;
        actionMount(event);
        setEventKey(OpenNewWindow);
        deviceListener->addSubscriber(this);
    }else{
        actionOpenInNewWindow(event);
    }
}

void AppController::asycOpenDiskInNewWindow(const QString &path)
{
    m_fmEvent << DUrl(path);
    actionOpenInNewWindow(m_fmEvent);
}

void AppController::actionOpenAsAdmain(const FMEvent &event)
{
    QStringList args;
    args << event.fileUrl().toLocalFile();
    qDebug() << args;
    QProcess::startDetached("dde-file-manager-pkexec", args);
}

void AppController::actionOpenWithCustom(const FMEvent &event)
{
    emit fileSignalManager->requestShowOpenWithDialog(event);
}

void AppController::actionOpenFileLocation(const FMEvent &event)
{
    const DUrlList& urls = event.fileUrlList();
    foreach (DUrl url, urls) {
        fileService->openFileLocation(url);
    }
}

void AppController::actionCompress(const FMEvent &event)
{
    const DUrlList& urls = event.fileUrlList();
    fileService->compressFiles(urls);
}

void AppController::actionDecompress(const FMEvent &event)
{
    const DUrlList &list = event.fileUrlList();
    fileService->decompressFile(list);

}

void AppController::actionDecompressHere(const FMEvent &event)
{
    const DUrlList &list = event.fileUrlList();
    fileService->decompressFileHere(list);
}

void AppController::actionCut(const FMEvent &event)
{
    const DUrlList& urls = event.fileUrlList();
    fileService->cutFiles(urls);

}

void AppController::actionCopy(const FMEvent &event)
{
    const DUrlList& urls = event.fileUrlList();
    fileService->copyFiles(urls);
}

void AppController::actionPaste(const FMEvent &event)
{
    fileService->pasteFile(event);
}

void AppController::actionRename(const FMEvent &event)
{
    if(event.parentSource() == FMEvent::LeftSideBar)
    {
        emit fileSignalManager->requestBookmarkRename(event);
        return;
    }

    emit fileSignalManager->requestRename(event);
}

void AppController::actionRemove(const FMEvent &event)
{
    const DUrlList& urls = event.fileUrlList();
    const DUrl& fileUrl = event.fileUrl();

    if (event.parentSource() == FMEvent::LeftSideBar) {
        fileSignalManager->requestBookmarkRemove(event);
    } else if (fileUrl.isRecentFile()) {
        fileSignalManager->requestRecentFileRemove(urls);
    }
}

void AppController::actionDelete(const FMEvent &event)
{
    const DUrlList& urls = event.fileUrlList();
    fileService->moveToTrash(urls);
}

void AppController::actionCompleteDeletion(const FMEvent &event)
{
    const DUrlList& urls = event.fileUrlList();
    fileService->deleteFiles(urls, event);
}

void AppController::actionCreateSymlink(const FMEvent &event)
{
//    const DUrl& fileUrl = event.fileUrl();
//    int windowId = event.windowId();
//    FileUtils::createSoftLink(windowId, fileUrl.toLocalFile());
    fileService->createSymlink(event.fileUrl(), event);
}

void AppController::actionSendToDesktop(const FMEvent &event)
{
    fileService->sendToDesktop(event);
}

void AppController::actionAddToBookMark(const FMEvent &event)
{
    const DUrl& fileUrl = event.fileUrl();
    QString dirName = QDir(fileUrl.toLocalFile()).dirName();
    bookmarkManager->writeIntoBookmark(0, dirName, fileUrl);
    emit fileSignalManager->requestBookmarkAdd(dirName, event);
}

void AppController::actionNewFolder(const FMEvent &event)
{
    fileService->newFolder(event);
}

void AppController::actionNewFile(const FMEvent &event)
{
    const DUrl& fileUrl = event.fileUrl();
    fileService->newFile(fileUrl);
}

void AppController::actionSelectAll(const FMEvent &event)
{
    int windowId = event.windowId();
    emit fileSignalManager->requestViewSelectAll(windowId);
}

void AppController::actionClearRecent(const FMEvent &event)
{
    Q_UNUSED(event)
    emit fileSignalManager->requestClearRecent();
}

void AppController::actionClearTrash(const FMEvent &event)
{
    DUrlList list;

    list << DUrl::fromLocalFile(DFMStandardPaths::standardLocation(DFMStandardPaths::TrashInfosPath)) << DUrl::fromLocalFile(DFMStandardPaths::standardLocation(DFMStandardPaths::TrashFilesPath));

    fileService->deleteFiles(list, event);


    SoundEffectInterface* soundEffectInterface = new SoundEffectInterface(SoundEffectInterface::staticServerPath(),
                                                                          SoundEffectInterface::staticInterfacePath(),
                                                                          QDBusConnection::sessionBus(), this);
    soundEffectInterface->PlaySystemSound("trash-empty");
    soundEffectInterface->deleteLater();
}

void AppController::actionNewWord(const FMEvent &event)
{
    const DUrl& fileUrl = event.fileUrl();
    int windowId = event.windowId();
    QString targetFile = FileUtils::newDocmentName(fileUrl.toLocalFile(), QObject::tr("newDoc"), "doc");
    AppController::selectionAndRenameFile = qMakePair(DUrl::fromLocalFile(targetFile), windowId);
    FileUtils::cpTemplateFileToTargetDir(fileUrl.toLocalFile(), QObject::tr("newDoc"), "doc");
}

void AppController::actionNewExcel(const FMEvent &event)
{
    const DUrl& fileUrl = event.fileUrl();
    int windowId = event.windowId();
    QString targetFile = FileUtils::newDocmentName(fileUrl.toLocalFile(), QObject::tr("newExcel"), "xls");
    AppController::selectionAndRenameFile = qMakePair(DUrl::fromLocalFile(targetFile), windowId);
    FileUtils::cpTemplateFileToTargetDir(fileUrl.toLocalFile(), QObject::tr("newExcel"), "xls");
}

void AppController::actionNewPowerpoint(const FMEvent &event)
{
    const DUrl& fileUrl = event.fileUrl();
    int windowId = event.windowId();
    QString targetFile = FileUtils::newDocmentName(fileUrl.toLocalFile(), QObject::tr("newPowerPoint"), "ppt");
    AppController::selectionAndRenameFile = qMakePair(DUrl::fromLocalFile(targetFile), windowId);
    FileUtils::cpTemplateFileToTargetDir(fileUrl.toLocalFile(), QObject::tr("newPowerPoint"), "ppt");
}

void AppController::actionNewText(const FMEvent &event)
{
    const DUrl& fileUrl = event.fileUrl();
    int windowId = event.windowId();
    QString targetFile = FileUtils::newDocmentName(fileUrl.toLocalFile(), QObject::tr("newText"), "txt");
    AppController::selectionAndRenameFile = qMakePair(DUrl::fromLocalFile(targetFile), windowId);
    FileUtils::cpTemplateFileToTargetDir(fileUrl.toLocalFile(), QObject::tr("newText"), "txt");
}

void AppController::actionMount(const FMEvent &event)
{
    const DUrl& fileUrl = event.fileUrl();
    deviceListener->mount(fileUrl.query());
}

void AppController::actionUnmount(const FMEvent &event)
{
    const DUrl& fileUrl = event.fileUrl();
    deviceListener->unmount(fileUrl.query(DUrl::FullyEncoded));
}

void AppController::actionRestore(const FMEvent &event)
{
    DUrlList urls;

    for (const DUrl &url : event.fileUrlList()) {
        if (url.isSearchFile())
            urls << url.searchedFileUrl();
        else
            urls << url;
    }

    emit fileSignalManager->requestRestoreTrashFile(urls, event);
}

void AppController::actionRestoreAll(const FMEvent &event)
{
    emit fileSignalManager->requestRestoreAllTrashFile(event);
}

void AppController::actionEject(const FMEvent &event)
{
    const DUrl& fileUrl = event.fileUrl();
    deviceListener->eject(fileUrl.query(DUrl::FullyEncoded));
}

void AppController::actionOpenInTerminal(const FMEvent &event)
{
    if (event.fileUrlList().isEmpty()) {
        fileService->openInTerminal(event.fileUrl());

        return;
    }

    for (const DUrl &url: event.fileUrlList()) {
        fileService->openInTerminal(url);
    }
}

void AppController::actionProperty(const FMEvent &event)
{
    if (event.fileUrlList().isEmpty())
        return;

    if (event.fileUrlList().first() == DUrl::fromTrashFile("/")){
        emit fileSignalManager->requestShowTrashPropertyDialog(event);
    }else{
        emit fileSignalManager->requestShowPropertyDialog(event);
    }
}

void AppController::actionNewWindow(const FMEvent &event)
{
    bool open_ok = false;

    for (const DUrl &fileUrl : event.fileUrlList()) {
        const AbstractFileInfoPointer &fileInfo = fileService->createFileInfo(fileUrl);

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

void AppController::actionHelp(const FMEvent &event)
{
    Q_UNUSED(event)
    QStringList args;
    args << qApp->applicationName();
    QProcess::startDetached("dman", args);
}

void AppController::actionAbout(const FMEvent &event)
{
    Q_UNUSED(event)
    emit fileSignalManager->showAboutDialog(event);
}

void AppController::actionExit(const FMEvent &event)
{
    int windowId = event.windowId();
    emit fileSignalManager->aboutToCloseLastActivedWindow(windowId);
}

void AppController::actionSetAsWallpaper(const FMEvent &event)
{
    const DUrl& fileUrl = event.fileUrl();
    FileUtils::setBackground(fileUrl.toLocalFile());
}

void AppController::actionUnShare(const FMEvent &event)
{
    const ShareInfo& info = userShareManager->getShareInfoByPath(event.fileUrl().path());
    userShareManager->deleteUserShare(info);
}

void AppController::actionctrlL(const FMEvent &event)
{
    emit fileSignalManager->requestSearchCtrlL(event);
}

void AppController::actionctrlF(const FMEvent &event)
{
    emit fileSignalManager->requestSearchCtrlF(event);
}

void AppController::actionExitCurrentWindow(const FMEvent &event)
{
    int windowId = event.windowId();
    WindowManager::getWindowById(windowId)->close();
}

void AppController::actionShowHotkeyHelp(const FMEvent &event)
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

void AppController::actionBack(const FMEvent &event)
{
    emit fileSignalManager->requestBack(event);
}

void AppController::actionForward(const FMEvent &event)
{
    emit fileSignalManager->requestForward(event);
}

void AppController::actionForgetPassword(const FMEvent &event)
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

void AppController::doSubscriberAction(const QString &path)
{
    switch (eventKey()) {
    case Open:
        asycOpenDisk(path);
        break;
    case OpenNewWindow:
        asycOpenDiskInNewWindow(path);
        break;
    default:
        break;
    }
    deviceListener->removeSubscriber(this);
}

AppController::~AppController()
{

}

