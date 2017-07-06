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
#include "bookmarkmanager.h"
#include "networkcontroller.h"
#include "deviceinfo/udisklistener.h"
#include "dfileservices.h"
#include "fileoperations/filejob.h"
#include "dfmeventdispatcher.h"

#include "app/filesignalmanager.h"
#include "dfmevent.h"
#include "app/define.h"

#include "interfaces/dfmstandardpaths.h"
#include "interfaces/dfmsetting.h"
#include "shutil/fileutils.h"
#include "views/windowmanager.h"
#include "views/dfilemanagerwindow.h"
#include "dbusinterface/commandmanager_interface.h"

#include "gvfs/networkmanager.h"
#include "gvfs/gvfsmountclient.h"
#include "gvfs/gvfsmountmanager.h"
#include "gvfs/secretmanager.h"
#include "usershare/usersharemanager.h"
#include "dialogs/dialogmanager.h"
#include "singleton.h"

#include "../deviceinfo/udisklistener.h"

#include <QProcess>
#include <QStorageInfo>
#include <QAction>
#include <DAboutDialog>
#include <qprocess.h>
#include <QMediaPlayer>
#include <QDBusObjectPath>
#include <QGSettings>
#include "shutil/shortcut.h"
#include "models/desktopfileinfo.h"

#ifdef SW_LABEL
#include "sw_label/filemanagerlibrary.h"
#endif

DWIDGET_USE_NAMESPACE

QPair<DUrl, quint64> AppController::selectionAndRenameFile;

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
    DFileService::dRegisterUrlHandler<ShareControler>(USERSHARE_SCHEME, "");
    DFileService::dRegisterUrlHandler<AVFSFileController>(AVFS_SCHEME, "");
}

void AppController::actionOpen(const QSharedPointer<DFMUrlListBaseEvent> &event)
{
    const DUrlList& urls = event->urlList();

    if (urls.isEmpty())
        return;

    if (urls.size() > 1 || globalSetting->isAllwayOpenOnNewWindow())
        DFMEventDispatcher::instance()->processEvent<DFMOpenUrlEvent>(event->sender(), urls, DFMOpenUrlEvent::ForceOpenNewWindow);
     else
        DFMEventDispatcher::instance()->processEventAsync<DFMOpenUrlEvent>(event->sender(), urls, DFMOpenUrlEvent::OpenInCurrentWindow);
}

void AppController::actionOpenDisk(const QSharedPointer<DFMUrlBaseEvent> &event)
{
    const DUrl& fileUrl = event->url();
    QString id = fileUrl.query();

    if (!id.isEmpty()){
        const QDiskInfo& diskInfo = gvfsMountManager->getDiskInfo(id);
        if (diskInfo.can_mount()){
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
    const DUrl& fileUrl = event->url();
    if (!QStorageInfo(fileUrl.toLocalFile()).isValid()){
        m_fmEvent = event;
        actionMount(event);
        setEventKey(OpenNewTab);
        deviceListener->addSubscriber(this);
    }else{
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
    const DUrl& fileUrl = event->url();
    if (!QStorageInfo(fileUrl.toLocalFile()).isValid()){
        m_fmEvent = event;
        actionMount(event);
        setEventKey(OpenNewWindow);
        deviceListener->addSubscriber(this);
    }else{
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
    const DUrlList& urls = event->urlList();
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

void AppController::actionRename(const QSharedPointer<DFMUrlBaseEvent> &event)
{
    emit fileSignalManager->requestRename(*event.data());
}

void AppController::actionBookmarkRename(const QSharedPointer<DFMUrlBaseEvent> &event)
{
    emit fileSignalManager->requestBookmarkRename(*event.data());
}

void AppController::actionBookmarkRemove(const QSharedPointer<DFMUrlBaseEvent> &event)
{
    emit fileSignalManager->requestBookmarkRemove(*event.data());
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
    const DUrl& fileUrl = event->url();
    QString dirName = QDir(fileUrl.path()).dirName();
    bookmarkManager->writeIntoBookmark(0, dirName, fileUrl);
    emit fileSignalManager->requestBookmarkAdd(dirName, DFMUrlBaseEvent(event->sender(), fileUrl));
}

void AppController::actionNewFolder(const QSharedPointer<DFMUrlBaseEvent> &event)
{
    fileService->newFolder(event->sender(), event->url());
}

void AppController::actionNewFile(const QSharedPointer<DFMUrlBaseEvent> &event)
{
    fileService->newFile(event->sender(), event->url(), "txt");
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

    if(ret){
        //check if is sound effect enabled
        QGSettings settings("com.deepin.dde.sound-effect", "/com/deepin/dde/sound-effect/");
        if(!settings.get("enabled").toBool())
            return;

        //check if is global sound off
        QDBusInterface audioIface("com.deepin.daemon.Audio",
                                  "/com/deepin/daemon/Audio",
                                  "com.deepin.daemon.Audio",
                                  QDBusConnection::sessionBus());

        QString defaultSink = qvariant_cast<QDBusObjectPath>(audioIface.property("DefaultSink")).path();

        QDBusInterface audioSinkIface("com.deepin.daemon.Audio",
                                      defaultSink,
                                      "com.deepin.daemon.Audio.Sink",
                                      QDBusConnection::sessionBus());
        bool isGlobalSoundDisabled = audioSinkIface.property("Mute").toBool();

        if(isGlobalSoundDisabled)
            return;

        QMediaPlayer* player = new QMediaPlayer;
        player->setMedia(QUrl::fromLocalFile("/usr/share/sounds/deepin/stereo/trash-empty.ogg"));
        player->setVolume(100);
        player->play();
        connect(player, &QMediaPlayer::positionChanged, [=](const qint64& position){
            if(position >= player->duration()){
                player->deleteLater();
            }
        });
    }
}

void AppController::actionNewWord(const QSharedPointer<DFMUrlBaseEvent> &event)
{
    const DUrl& fileUrl = event->url();
    int windowId = event->windowId();
    QString targetFile = FileUtils::newDocmentName(fileUrl.toLocalFile(), QObject::tr("Document"), "doc");
    AppController::selectionAndRenameFile = qMakePair(DUrl::fromLocalFile(targetFile), windowId);
    FileUtils::cpTemplateFileToTargetDir(fileUrl.toLocalFile(), QObject::tr("Document"), "doc");
}

void AppController::actionNewExcel(const QSharedPointer<DFMUrlBaseEvent> &event)
{
    const DUrl& fileUrl = event->url();
    int windowId = event->windowId();
    QString targetFile = FileUtils::newDocmentName(fileUrl.toLocalFile(), QObject::tr("Spreadsheet"), "xls");
    AppController::selectionAndRenameFile = qMakePair(DUrl::fromLocalFile(targetFile), windowId);
    FileUtils::cpTemplateFileToTargetDir(fileUrl.toLocalFile(), QObject::tr("Spreadsheet"), "xls");
}

void AppController::actionNewPowerpoint(const QSharedPointer<DFMUrlBaseEvent> &event)
{
    const DUrl& fileUrl = event->url();
    int windowId = event->windowId();
    QString targetFile = FileUtils::newDocmentName(fileUrl.toLocalFile(), QObject::tr("Presentation"), "ppt");
    AppController::selectionAndRenameFile = qMakePair(DUrl::fromLocalFile(targetFile), windowId);
    FileUtils::cpTemplateFileToTargetDir(fileUrl.toLocalFile(), QObject::tr("Presentation"), "ppt");
}

void AppController::actionNewText(const QSharedPointer<DFMUrlBaseEvent> &event)
{
    const DUrl& fileUrl = event->url();
    int windowId = event->windowId();
    QString targetFile = FileUtils::newDocmentName(fileUrl.toLocalFile(), QObject::tr("Text"), "txt");
    AppController::selectionAndRenameFile = qMakePair(DUrl::fromLocalFile(targetFile), windowId);
    FileUtils::cpTemplateFileToTargetDir(fileUrl.toLocalFile(), QObject::tr("Text"), "txt");
}

void AppController::actionMount(const QSharedPointer<DFMUrlBaseEvent> &event)
{
    const DUrl& fileUrl = event->url();
    deviceListener->mount(fileUrl.query());
}

void AppController::actionUnmount(const QSharedPointer<DFMUrlBaseEvent> &event)
{
    const DUrl& fileUrl = event->url();
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
    const DUrl& fileUrl = event->url();
    deviceListener->eject(fileUrl.query(DUrl::FullyEncoded));
}

void AppController::actionOpenInTerminal(const QSharedPointer<DFMUrlListBaseEvent> &event)
{
    for (const DUrl &url: event->urlList()) {
        fileService->openInTerminal(event->sender(), url);
    }
}

void AppController::actionProperty(const QSharedPointer<DFMUrlListBaseEvent> &event)
{
    DUrlList urlList = event->urlList();

    foreach (const DUrl& url, urlList) {
        DUrl realTargetUrl = url;

        //consider symlink file that links to trash/computer desktop files
        const DAbstractFileInfoPointer& info = fileService->createFileInfo(event->sender(), url);
        if(info && info->isSymLink()){
            realTargetUrl = info->rootSymLinkTarget();
        }

        if(realTargetUrl.toLocalFile().endsWith(QString(".") + DESKTOP_SURRIX)){
            DesktopFile df(realTargetUrl.toLocalFile());
            if(df.getDeepinId() == "dde-trash"){
                dialogManager->showTrashPropertyDialog(DFMUrlBaseEvent(event->sender(), realTargetUrl));
                urlList.removeOne(url);
            } else if(df.getDeepinId() == "dde-computer"){
                dialogManager->showComputerPropertyDialog();
                urlList.removeOne(url);
            }
        }
    }

    if(urlList.isEmpty())
        return;

    if (urlList.first() == DUrl::fromTrashFile("/")) {
        emit fileSignalManager->requestShowTrashPropertyDialog(DFMUrlBaseEvent(event->sender(), urlList.first()));
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
    QStringList args;
    args << qApp->applicationName();
    QProcess::startDetached("dman", args);
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
    const DUrl& fileUrl = event->url();

    if (fileUrl.isLocalFile())
        FileUtils::setBackground(fileUrl.toLocalFile());
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
    QWidget* w = WindowManager::getWindowById(event->windowId());
    if(!w)
        return;

    UDiskDeviceInfoPointer info = deviceListener->getDeviceByDeviceID(event->url().query());
    if(!info)
        return;

    QString devicePath = info->getPath();

    QString cmd = "usb-device-formatter-pkexec";
    QStringList args;
    args << "-m="+QString::number(event->windowId()) <<devicePath;

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
    QRect rect=WindowManager::getWindowById(winId)->geometry();
    QPoint pos(rect.x() + rect.width()/2 , rect.y() + rect.height()/2);
    Shortcut sc;
    QStringList args;
    QString param1 = "-j="+sc.toStr();
    QString param2 = "-p=" + QString::number(pos.x()) + "," + QString::number(pos.y());
    args<<param1<<param2;
    QProcess::startDetached("deepin-shortcut-viewer",args);
}

void AppController::actionBack(quint64 winId)
{
    DFMEventDispatcher::instance()->processEvent(dMakeEventPointer<DFMBackEvent>(this), qobject_cast<DFileManagerWindow*>(WindowManager::getWindowById(winId)));
}

void AppController::actionForward(quint64 winId)
{
    DFMEventDispatcher::instance()->processEvent(dMakeEventPointer<DFMForwardEvent>(this), qobject_cast<DFileManagerWindow*>(WindowManager::getWindowById(winId)));
}

void AppController::actionForgetPassword(const QSharedPointer<DFMUrlBaseEvent> &event)
{
    QString path = event->url().query();

    QJsonObject smbObj = secretManager->getLoginData(path);
    if (smbObj.empty()){
        if (path.endsWith("/")){
            path = path.left(path.length() - 1);
            smbObj = secretManager->getLoginData(path);

            if (smbObj.isEmpty()){
                QString share = path.split("/").last();
                path = path.left(path.length() - share.length());
                path += share.toUpper();
                smbObj = secretManager->getLoginData(path);
            }
        }else{
            path += "/";
            smbObj = secretManager->getLoginData(path);
            if (smbObj.isEmpty()){
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
        obj.insert("protocol", DUrl(smbObj.value("id").toString()).scheme());
        obj.insert("server", server);
        secretManager->clearPasswordByLoginObj(obj);
    }
    actionUnmount(event);
}

void AppController::actionOpenFileByApp()
{
    const QAction *action = qobject_cast<QAction*>(sender());

    if (!action)
        return;

    QString app = action->property("app").toString();
    DUrl fileUrl = qvariant_cast<DUrl>(action->property("url"));
    fileService->openFileByApp(this, app, fileUrl);
}


#ifdef SW_LABEL
void AppController::actionSetLabel(const DFMEvent &event)
{
    if (FileManagerLibrary::instance()->isCompletion()){
        std::string path = event.fileUrl().toLocalFile().toStdString();
//        auto_operation(const_cast<char*>(path.c_str()), "020100");
        FileManagerLibrary::instance()->auto_operation()(const_cast<char*>(path.c_str()), "020100");
        qDebug() << "020100" << "set label";
    }
}

void AppController::actionViewLabel(const DFMEvent &event)
{
    if (FileManagerLibrary::instance()->isCompletion()){
        std::string path = event.fileUrl().toLocalFile().toStdString();
//        auto_operation(const_cast<char*>(path.c_str()), "010101");
        FileManagerLibrary::instance()->auto_operation()(const_cast<char*>(path.c_str()), "010101");
        qDebug() << "010101" << "view label";
    }
}

void AppController::actionEditLabel(const DFMEvent &event)
{
    if (FileManagerLibrary::instance()->isCompletion()){
        std::string path = event.fileUrl().toLocalFile().toStdString();
//        auto_operation(const_cast<char*>(path.c_str()), "010201");
        FileManagerLibrary::instance()->auto_operation()(const_cast<char*>(path.c_str()), "010201");
        qDebug() << "010201" << "edit label";
    }
}

void AppController::actionPrivateFileToPublic(const DFMEvent &event)
{
    if (FileManagerLibrary::instance()->isCompletion()){
        std::string path = event.fileUrl().toLocalFile().toStdString();
//        auto_operation(const_cast<char*>(path.c_str()), "010501");
        FileManagerLibrary::instance()->auto_operation()(const_cast<char*>(path.c_str()), "010501");
        qDebug() << "010501" << "private file to public";
    }
}

void AppController::actionByIds(const DFMEvent &event, QString actionId)
{
    if (FileManagerLibrary::instance()->isCompletion()){
        std::string path = event.fileUrl().toLocalFile().toStdString();
        std::string _actionId = actionId.toStdString();
        FileManagerLibrary::instance()->auto_operation()(const_cast<char*>(path.c_str()), const_cast<char*>(_actionId.c_str()));
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

    if (!info.exists())
        return QString();

    const QString &targetFile = FileUtils::newDocmentName(targetDir, baseFileName, info.suffix());
    AppController::selectionAndRenameFile = qMakePair(DUrl::fromLocalFile(targetFile), windowId);

    if (QFile::copy(sourceFile, targetFile))
        return targetFile;

    return QString();
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
    secretManager;
}

void AppController::createUserShareManager()
{
    userShareManager;
}
