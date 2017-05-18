#include "filemanagerapp.h"
#include "app/filesignalmanager.h"
#include "dfmglobal.h"
#include "views/windowmanager.h"
#include "views/dfilemanagerwindow.h"

#include "controllers/appcontroller.h"
#include "controllers/pathmanager.h"
#include "app/define.h"

#include "dialogs/dialogmanager.h"

#include "models/fmstate.h"

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
#include "interfaces/dfmsetting.h"
#include "shutil/fileutils.h"
#include "utils/utils.h"

#include <QDataStream>
#include <QGuiApplication>
#include <QTimer>
#include <QThreadPool>
#include <QSettings>
#include <QFileSystemWatcher>
#include <QProcess>

class FileManagerAppGlobal : public FileManagerApp {};
Q_GLOBAL_STATIC(FileManagerAppGlobal, fmaGlobal)

FileManagerApp::FileManagerApp(QObject *parent) : QObject(parent)
{
    initApp();
    initView();
    lazyRunInitServiceTask();
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
    /*init configure*/
    DFMGlobal::initGlobalSettings();

    /*add menuextensions path*/
    DFMGlobal::autoLoadDefaultMenuExtensions();

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

    /*init gvfsMountClient */
    DFMGlobal::initGvfsMountClient();

    /*init gvfsMountClient */
    DFMGlobal::initGvfsMountManager();
#ifdef AUTOMOUNT
    gvfsMountManager->setAutoMountSwitch(true);
#endif

    /*init secretManger */
    DFMGlobal::initSecretManager();

    /*init userShareManager */
    DFMGlobal::initUserShareManager();

    /*init controllers for different scheme*/
    fileService->initHandlersByCreators();

    /*init viewStatesManager*/
    DFMGlobal::initViewStatesManager();

    QThreadPool::globalInstance()->setMaxThreadCount(MAX_THREAD_COUNT);
}

void FileManagerApp::initView()
{
    m_windowManager = WindowManager::instance();
}

void FileManagerApp::initManager()
{

}

void FileManagerApp::initTranslation()
{

}

void FileManagerApp::lazyRunInitServiceTask()
{
    QTimer::singleShot(1500, initService);
}

void FileManagerApp::initSysPathWatcher()
{
    m_sysPathWatcher = new QFileSystemWatcher(this);
    m_sysPathWatcher->addPath(QDir::homePath());
}

void FileManagerApp::initConnect()
{
    connect(m_sysPathWatcher, &QFileSystemWatcher::directoryChanged, systemPathManager, &PathManager::loadSystemPaths);
}

void FileManagerApp::initService()
{
    if (globalSetting->isCompressFilePreview()){
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

