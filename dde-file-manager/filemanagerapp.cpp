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
    lazyRunCacheTask();

    QFileSystemWatcher *watcherHome = new QFileSystemWatcher(this);

    connect(watcherHome, &QFileSystemWatcher::directoryChanged, systemPathManager, &PathManager::loadSystemPaths);

    watcherHome->addPath(QDir::homePath());
}

FileManagerApp::~FileManagerApp()
{

}


void FileManagerApp::initApp()
{
    /// init dialog manager
    DFMGlobal::initDialogManager();

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
    connect(fileSignalManager, &FileSignalManager::requestUpdateMimeAppsCache, mimeAppsManager, &MimesAppsManager::requestUpdateCache);
}

QString FileManagerApp::getFileJobConfigPath()
{
    return QString("%1/filejob.conf").arg(StandardPath::getConfigPath());
}

void FileManagerApp::show(const DUrl &url)
{
    m_windowManager->showNewWindow(url);
    m_taskTimer->start();
}

void FileManagerApp::lazyRunCacheTask()
{
    m_taskTimer = new QTimer;
    m_taskTimer->setSingleShot(true);
    m_taskTimer->setInterval(2000);
    connect(m_taskTimer, &QTimer::timeout, fileSignalManager, &FileSignalManager::requestUpdateMimeAppsCache);
    connect(m_taskTimer, &QTimer::timeout, m_taskTimer, &QTimer::deleteLater);
}

void FileManagerApp::runCacheTask()
{
    emit fileSignalManager->requestUpdateMimeAppsCache();
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
