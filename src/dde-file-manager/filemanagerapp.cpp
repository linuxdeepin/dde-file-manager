// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filemanagerapp.h"
#include "dfmglobal.h"
#include "dfmapplication.h"

#include "views/windowmanager.h"
#include "views/dfilemanagerwindow.h"

#include "controllers/appcontroller.h"
#include "controllers/pathmanager.h"
#include "app/define.h"
#include "app/filesignalmanager.h"

#include "dialogs/dialogmanager.h"

#include "shutil/mimesappsmanager.h"
#include "interfaces/dfmstandardpaths.h"
#include "gvfs/gvfsmountmanager.h"
#include "interfaces/dfileinfo.h"

#include "singleton.h"
#include "commandlinemanager.h"
#include "interfaces/durl.h"
#include "plugins/pluginmanager.h"
#include "interfaces/dfmstandardpaths.h"
#include "models/desktopfileinfo.h"
#include "interfaces/dfileservices.h"
#include "shutil/fileutils.h"
#include "utils/utils.h"
#include "utils/rlog/rlog.h"
#include "app/filesignalmanager.h"

#include "tag/tagmanager.h"

#include <QDataStream>
#include <QGuiApplication>
#include <QTimer>
#include <QThreadPool>
#include <QFileSystemWatcher>
#include <QProcess>

class FileManagerAppGlobal : public FileManagerApp {};
Q_GLOBAL_STATIC(FileManagerAppGlobal, fmaGlobal)

FileManagerApp::FileManagerApp(QObject *parent) : QObject(parent)
{
    initApp();
    initView();
    lazyRunTask();
    initSysPathWatcher();
    initConnect();
}

FileManagerApp *FileManagerApp::instance()
{
    return fmaGlobal;
}

FileManagerApp::~FileManagerApp()
{

}

void FileManagerApp::initApp()
{
    qDebug() << FileUtils::getKernelParameters();

    /*add plugin path*/
    DFMGlobal::autoLoadDefaultPlugins();

    /*init plugin manager */
    DFMGlobal::initPluginManager();

    /*init searchHistoryManager */
    DFMGlobal::initSearchHistoryManager();

    /*init bookmarkManager */
    DFMGlobal::initBookmarkManager();

    /*init fileMenuManager */
    DFMGlobal::initFileMenuManager();

    /*init fileSignalManger */
    DFMGlobal::initFileSiganlManager();

    /* init dialog manager */
    DFMGlobal::initDialogManager();

    /*init appController */
    DFMGlobal::initAppcontroller();

    /*init fileService */
    DFMGlobal::initFileService();

    /*init deviceListener */
    DFMGlobal::initDeviceListener();

    /*init mimeAppsManager*/
    DFMGlobal::initMimesAppsManager();

    /*init systemPathMnager */
    DFMGlobal::initSystemPathManager();

    /*init mimeTypeDisplayManager */
    DFMGlobal::initMimeTypeDisplayManager();

    /*init networkManager */
    DFMGlobal::initNetworkManager();

    /*init gvfsMountManager */
    DFMGlobal::initGvfsMountManager();

    /*init secretManger */
    DFMGlobal::initSecretManager();

    /*init userShareManager */
    DFMGlobal::initUserShareManager();

    /*init controllers for different scheme*/
    fileService->initHandlersByCreators();

    /*init operator revocation*/
    DFMGlobal::initOperatorRevocation();

    DFMGlobal::initTagManagerConnect();

    /*init thumbnail connection*/
    DFMGlobal::initThumbnailConnection();

    /*init bluetooth manager*/
    DFMGlobal::initBluetoothManager();

    /*init rootfile manager*/
    DFMGlobal::initRootFileManager();

    /*init emblemplugin manager*/
    DFMGlobal::initEmblemPluginManagerConnection();

    DFMGlobal::initRlogManager();

    QThreadPool::globalInstance()->setMaxThreadCount(MAX_THREAD_COUNT);
}

void FileManagerApp::initView()
{
    m_windowManager = WindowManager::instance();
    m_windowManager->setEnableAutoQuit(true);
}

void FileManagerApp::initManager()
{

}

void FileManagerApp::initTranslation()
{

}

void FileManagerApp::lazyRunTask()
{
    QTimer::singleShot(1500, initService);

    // report app startup event
    QTimer::singleShot(500, this, [=](){
        QVariantMap data;
        data.insert("type", true);
        rlog->commit("AppStartup", data);
    });
}

void FileManagerApp::initSysPathWatcher()
{
    m_sysPathWatcher = new QFileSystemWatcher(this);
    m_sysPathWatcher->addPath(QDir::homePath());
}

void FileManagerApp::initConnect()
{
    connect(m_sysPathWatcher, &QFileSystemWatcher::directoryChanged, systemPathManager, &PathManager::loadSystemPaths);
    connect(DFMApplication::instance(), &DFMApplication::previewCompressFileChanged, this, [] (bool enable) {
        if (enable)
            FileUtils::mountAVFS();
        else
            FileUtils::umountAVFS();
    });
}

void FileManagerApp::initService()
{
    if (DFMApplication::instance()->genericAttribute(DFMApplication::GA_PreviewCompressFile).toBool()) {
        FileUtils::mountAVFS();
    }
}

void FileManagerApp::show(const DUrl &url)
{
    m_windowManager->showNewWindow(url);
}

void FileManagerApp::showPropertyDialog(const QStringList paths)
{
    DUrlList urlList;
    foreach (QString path, paths) {
        DUrl url = DUrl::fromUserInput(path);
        QString uPath = url.path();
        if(uPath.endsWith(QDir::separator()) && uPath.size() > 1)
            uPath.chop(1);
        url.setPath(uPath);

        //symlink , desktop files filters
        const DAbstractFileInfoPointer& info = fileService->createFileInfo(this, url);
        DUrl realTargetUrl = url;
        if(info && info->isSymLink()){
            realTargetUrl = info->rootSymLinkTarget();
        }

        if(realTargetUrl.toLocalFile().endsWith(".desktop")){
            DesktopFile df(realTargetUrl.toLocalFile());
            if(df.getDeepinId() == "dde-trash"){
                DFMEvent event(this);
                event.setData(DUrl::fromTrashFile("/"));
                dialogManager->showTrashPropertyDialog(event);
                continue;
            } else if(df.getDeepinId() == "dde-computer"){
                dialogManager->showComputerPropertyDialog();
                continue;
            }
        }

        //trash:/// and computer:///
        if(url == DUrl::fromComputerFile("/")){
            dialogManager->showComputerPropertyDialog();
            continue;
        }
        if(url == DUrl::fromTrashFile("/")){
            DFMEvent event(this);
            event.setData(DUrl::fromTrashFile("/"));
            dialogManager->showTrashPropertyDialog(event);
            continue;
        }
        if (!url.scheme().isEmpty()){
            if(url.scheme() == FILE_SCHEME && !QFile::exists(url.path()))
                continue;
            if(url == DUrl::fromTrashFile("/") ||
                    url == DesktopFileInfo::trashDesktopFileUrl()){
                DFMEvent event(this);
                DUrlList urls;
                urls << url;
                event.setData(urls);
                emit fileSignalManager->requestShowTrashPropertyDialog(event);
                continue;
            }
        }else{
            if(!QFile::exists(DUrl::fromLocalFile(path).path()))
                continue;
        }

        if(urlList.contains(url))
            continue;
        urlList << url;
    }
    if(urlList.isEmpty())
        return;

    emit fileSignalManager->requestShowPropertyDialog(DFMUrlListBaseEvent(this, urlList));
}

void FileManagerApp::openWithDialog(const QStringList files)
{
    DUrlList urlList;
     foreach (QString path, files) {
         DUrl url = DUrl::fromUserInput(path);
         QString uPath = url.path();
         if(uPath.endsWith(QDir::separator()) && uPath.size() > 1)
             uPath.chop(1);
         url.setPath(uPath);

         //symlink , desktop files filters
         const DAbstractFileInfoPointer& info = fileService->createFileInfo(this, url);
         DUrl realTargetUrl = url;
         if(info && info->isSymLink()){
             realTargetUrl = info->rootSymLinkTarget();
         }

         if(realTargetUrl.toLocalFile().endsWith(".desktop")){
             DesktopFile df(realTargetUrl.toLocalFile());
             if(df.getDeepinId() == "dde-trash"){
                 DFMEvent event(this);
                 event.setData(DUrl::fromTrashFile("/"));
                 dialogManager->showTrashPropertyDialog(event);
                 continue;
             } else if(df.getDeepinId() == "dde-computer"){
                 dialogManager->showComputerPropertyDialog();
                 continue;
             }
         }

         //trash:/// and computer:///
         if(url == DUrl::fromComputerFile("/")){
             dialogManager->showComputerPropertyDialog();
             continue;
         }
         if(url == DUrl::fromTrashFile("/")){
             DFMEvent event(this);
             event.setData(DUrl::fromTrashFile("/"));
             dialogManager->showTrashPropertyDialog(event);
             continue;
         }
         if (!url.scheme().isEmpty()){
             if(url.scheme() == FILE_SCHEME && !QFile::exists(url.path()))
                 continue;
             if(url == DUrl::fromTrashFile("/") ||
                     url == DesktopFileInfo::trashDesktopFileUrl()){
                 DFMEvent event(this);
                 DUrlList urls;
                 urls << url;
                 event.setData(urls);
                 emit fileSignalManager->requestShowTrashPropertyDialog(event);
                 continue;
             }
         }else{
             if(!QFile::exists(DUrl::fromLocalFile(path).path()))
                 continue;
         }

         if(urlList.contains(url))
             continue;
         urlList << url;
     }
     if(urlList.isEmpty())
         return;

    emit fileSignalManager->requestShowOpenFilesWithDialog(DFMUrlListBaseEvent(this, urlList));
}
