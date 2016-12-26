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
#include "../shutil/standardpath.h"
#include "fileoperations/filejob.h"

#include "widgets/singleton.h"
#include "widgets/commandlinemanager.h"
#include "interfaces/durl.h"
#include "plugins/pluginmanager.h"
#include "interfaces/dfmstandardpaths.h"
#include "models/trashdesktopfileinfo.h"
#include "models/computerdesktopfileinfo.h"
#include "interfaces/dfileservices.h"

#if QT_VERSION_MINOR < 6
#include "xdnd/xdndworkaround.h"
#endif

#include <QDataStream>
#include <QGuiApplication>
#include <QTimer>
#include <QThreadPool>
#include <QSettings>
#include <QFileSystemWatcher>

FileManagerApp::FileManagerApp(QObject *parent) : QObject(parent)
{
    initApp();
    initView();
    initConnect();
//    lazyRunCacheTask();

    QFileSystemWatcher *watcherHome = new QFileSystemWatcher(this);

    connect(watcherHome, &QFileSystemWatcher::directoryChanged, systemPathManager, &PathManager::loadSystemPaths);

    watcherHome->addPath(QDir::homePath());
}

FileManagerApp::~FileManagerApp()
{

}


void FileManagerApp::initApp()
{
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

    /*init iconProvider */
    DFMGlobal::initIconProvider();

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

    /*init thumbnailManager */
    DFMGlobal::initThumbnailManager();

    /*init networkManager */
    DFMGlobal::initNetworkManager();

    /*init gvfsMountClient */
    DFMGlobal::initGvfsMountClient();

    /*init secretManger */
    DFMGlobal::initSecretManager();

    /*init userShareManager */
    DFMGlobal::initUserShareManager();

    /*init controllers for different scheme*/
    fileService->initHandlersByCreators();

#if QT_VERSION_MINOR < 6
    /// fix Qt drag drop to google chrome bug
    new XdndWorkaround();
#endif

    QThreadPool::globalInstance()->setMaxThreadCount(MAX_THREAD_COUNT);
}

void FileManagerApp::initView()
{
    m_windowManager = new WindowManager;
}

void FileManagerApp::initManager()
{

}

void FileManagerApp::initTranslation()
{

}

void FileManagerApp::initConnect()
{
//    connect(fileSignalManager, &FileSignalManager::requestUpdateMimeAppsCache, mimeAppsManager, &MimesAppsManager::requestUpdateCache);
}

QString FileManagerApp::getFileJobConfigPath()
{
    return QString("%1/filejob.conf").arg(StandardPath::getConfigPath());
}

void FileManagerApp::show(const DUrl &url)
{
    m_windowManager->showNewWindow(url);
}

void FileManagerApp::loadFileJobConfig()
{
    QString configPath = getFileJobConfigPath();
    if (QFile(configPath).exists()){
        QSettings filejobSettings(configPath, QSettings::IniFormat);
        filejobSettings.beginGroup("FileJob");
        FileJob::Msec_For_Display = filejobSettings.value("Msec_For_Display", 1000).toLongLong();
        FileJob::Data_Block_Size = filejobSettings.value("Data_Block_Size", 65536).toLongLong();
        FileJob::Data_Flush_Size = filejobSettings.value("Data_Flush_Size", 16777216).toLongLong();
        filejobSettings.endGroup();
    }
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
        if(url == ComputerDesktopFileInfo::computerDesktopFileUrl())
            continue;
        if (!url.scheme().isEmpty()){
            if(url.scheme() == FILE_SCHEME && !QFile::exists(url.path()))
                continue;
            if(url == DUrl::fromTrashFile("/") ||
                    url == TrashDesktopFileInfo::trashDesktopFileUrl()){
                DFMEvent event;
                event << DUrl::fromTrashFile("/");
                DUrlList urls;
                urls << url;
                event << urls;
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
    DFMEvent event;
    event << urlList.first();
    event << urlList;

    emit fileSignalManager->requestShowPropertyDialog(event);
}
