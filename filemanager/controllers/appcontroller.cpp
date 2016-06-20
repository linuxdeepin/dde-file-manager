#include "appcontroller.h"
#include "movejobcontroller.h"
#include "trashjobcontroller.h"
#include "copyjobcontroller.h"
#include "deletejobcontroller.h"
#include "filecontroller.h"
#include "recenthistorymanager.h"
#include "trashmanager.h"
#include "searchcontroller.h"
#include "bookmarkmanager.h"
#include "networkcontroller.h"
#include "deviceinfo/udisklistener.h"
#include "../app/global.h"
#include "../app/fmevent.h"


#include "../shutil/fileutils.h"
#include "../app/filesignalmanager.h"
#include "fileservices.h"
#include "filejob.h"
#include "../views/windowmanager.h"
#include <QProcess>
#include <QStorageInfo>
#include <DAboutDialog>

AppController::AppController(QObject *parent) : QObject(parent)
{
    FileServices::instance()->setFileUrlHandler(RECENT_SCHEME, "", new RecentHistoryManager(this));
    FileServices::dRegisterUrlHandler<FileController>(FILE_SCHEME, "");
    FileServices::dRegisterUrlHandler<TrashManager>(TRASH_SCHEME, "");
    FileServices::dRegisterUrlHandler<SearchController>(SEARCH_SCHEME, "");
    FileServices::dRegisterUrlHandler<NetworkController>(NETWORK_SCHEME, "");
    FileServices::dRegisterUrlHandler<NetworkController>(SMB_SCHEME, "");
    networkManager;
    gvfsMountClient;
}

void AppController::initConnect()
{

}

void AppController::actionOpen(const FMEvent &event)
{
    const DUrlList& urls = event.fileUrlList();
    if (urls.size() == 1){
        fileService->openUrl(event);
    }else{
        foreach (DUrl url, urls) {
            if (url.isRecentFile()){
                url.setScheme(FILE_SCHEME);
            }
            if (url.isLocalFile()){
                QFileInfo info(url.toLocalFile());
                if (info.isFile()){
                    const_cast<FMEvent&>(event) = url;
                    fileService->openUrl(event);
                }else if(info.isDir()){
                    emit fileSignalManager->requestOpenNewWindowByUrl(url, true);
                }
            }
        }
    }
}

void AppController::actionOpenDisk(const FMEvent &event)
{

    const DUrl& fileUrl = event.fileUrl();
    if (!QStorageInfo(fileUrl.toLocalFile()).isValid()){
        qDebug() << event;
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
    m_fmEvent = DUrl(path);
    actionOpen(m_fmEvent);
}

void AppController::actionOpenInNewWindow(const FMEvent &event)
{
    const DUrl& fileUrl = event.fileUrl();
    fileService->openNewWindow(fileUrl);
}

void AppController::actionOpenDiskInNewWindow(const FMEvent &event)
{
    qDebug() << event;
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
    m_fmEvent = DUrl(path);
    actionOpenInNewWindow(m_fmEvent);
}

void AppController::actionOpenWithCustom(const FMEvent &event)
{
    emit fileSignalManager->requestShowOpenWithDialog(event);
}

void AppController::actionOpenFileLocation(const FMEvent &event)
{
    const DUrlList& urls = event.fileUrlList();
    foreach (DUrl url, urls) {
        url.setScheme(FILE_SCHEME);
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
    const DUrl& fileUrl = event.fileUrl();
    fileService->decompressFile(fileUrl);
}

void AppController::actionDecompressHere(const FMEvent &event)
{
    const DUrl& fileUrl = event.fileUrl();
    fileService->decompressFileHere(fileUrl);
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
    if(fileUrl.isLocalFile()) {
        fileSignalManager->requestBookmarkRemove(event);
    } else if(fileUrl.isRecentFile()) {
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

void AppController::actionCreateSoftLink(const FMEvent &event)
{
    const DUrl& fileUrl = event.fileUrl();
    int windowId = event.windowId();
    FileUtils::createSoftLink(windowId, fileUrl.toLocalFile());
}

void AppController::actionSendToDesktop(const FMEvent &event)
{
    const DUrlList& urls = event.fileUrlList();
    FileUtils::sendToDesktop(urls);
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
    fileService->deleteFiles(DUrlList() << DUrl::fromLocalFile(TRASHPATH), event);
}

void AppController::actionNewWord(const FMEvent &event)
{
    const DUrl& fileUrl = event.fileUrl();
    int windowId = event.windowId();
    QString targetFile = FileUtils::newDocmentName(fileUrl.toLocalFile(), QObject::tr("newDoc"), "doc");
    FileJob::SelectedFiles.insert(DUrl::fromLocalFile(targetFile), windowId);
    FileUtils::cpTemplateFileToTargetDir(fileUrl.toLocalFile(), QObject::tr("newDoc"), "doc");
}

void AppController::actionNewExcel(const FMEvent &event)
{
    const DUrl& fileUrl = event.fileUrl();
    int windowId = event.windowId();
    QString targetFile = FileUtils::newDocmentName(fileUrl.toLocalFile(), QObject::tr("newExcel"), "xls");
    FileJob::SelectedFiles.insert(DUrl::fromLocalFile(targetFile), windowId);
    FileUtils::cpTemplateFileToTargetDir(fileUrl.toLocalFile(), QObject::tr("newExcel"), "xls");
}

void AppController::actionNewPowerpoint(const FMEvent &event)
{
    const DUrl& fileUrl = event.fileUrl();
    int windowId = event.windowId();
    QString targetFile = FileUtils::newDocmentName(fileUrl.toLocalFile(), QObject::tr("newPowerPoint"), "ppt");
    FileJob::SelectedFiles.insert(DUrl::fromLocalFile(targetFile), windowId);
    FileUtils::cpTemplateFileToTargetDir(fileUrl.toLocalFile(), QObject::tr("newPowerPoint"), "ppt");
}

void AppController::actionNewText(const FMEvent &event)
{
    const DUrl& fileUrl = event.fileUrl();
    int windowId = event.windowId();
    QString targetFile = FileUtils::newDocmentName(fileUrl.toLocalFile(), QObject::tr("newText"), "txt");
    FileJob::SelectedFiles.insert(DUrl::fromLocalFile(targetFile), windowId);
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
    deviceListener->unmount(fileUrl.query());
}

void AppController::actionRestore(const FMEvent &event)
{
    const DUrlList& urls = event.fileUrlList();
    emit fileSignalManager->requestRestoreTrashFile(urls, event);
}

void AppController::actionRestoreAll(const FMEvent &event)
{
    emit fileSignalManager->requestRestoreAllTrashFile(event);
}

void AppController::actionEject(const FMEvent &event)
{
    const DUrl& fileUrl = event.fileUrl();
    deviceListener->eject(fileUrl.query());
}

void AppController::actionOpenInTerminal(const FMEvent &event)
{
    const DUrl& fileUrl = event.fileUrl();
    QStringList args;
    args << QString("--working-directory=%1").arg(fileUrl.toLocalFile());
    QProcess::startDetached("x-terminal-emulator", args);
}

void AppController::actionProperty(const FMEvent &event)
{
    emit fileSignalManager->requestShowPropertyDialog(event);
}

void AppController::actionNewWindow(const FMEvent &event)
{
    const DUrl& fileUrl = event.fileUrl();
    fileService->openNewWindow(fileUrl);
}

void AppController::actionHelp(const FMEvent &event)
{
    QStringList args;
    args << "dde-file-manager";
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
}

void AppController::actionBack(const FMEvent &event)
{
    emit fileSignalManager->requestBack(event);
}

void AppController::actionForward(const FMEvent &event)
{
    emit fileSignalManager->requestForward(event);
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

