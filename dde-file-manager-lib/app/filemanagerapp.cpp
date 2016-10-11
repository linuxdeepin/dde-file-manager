#include "filemanagerapp.h"
#include "filesignalmanager.h"

#include "views/windowmanager.h"
#include "views/dfilemanagerwindow.h"

#include "controllers/appcontroller.h"

#include "app/define.h"
#include "app/global.h"

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

#include <QLocalServer>
#include <QLocalSocket>
#include <QDataStream>
#include <QGuiApplication>
#include <QTimer>
#include <QThreadPool>
#include <QSettings>


FileManagerApp::FileManagerApp(QObject *parent) : QObject(parent)
{
    initApp();
    initView();
    initController();
    initGtk();
    initConnect();
    lazyRunCacheTask();
}

FileManagerApp::~FileManagerApp()
{

}


void FileManagerApp::initApp()
{
    /// init dialog manager
    dialogManager;

#if QT_VERSION_MINOR < 6
    /// fix Qt drag drop to google chrome bug
    new XdndWorkaround();
#endif

    QThreadPool::globalInstance()->setMaxThreadCount(MAX_THREAD_COUNT);

    QFont font;
    font.setPixelSize(14);
    qApp->setFont(font);
//#ifndef QT_DEBUG
//    qApp->setQuitOnLastWindowClosed(false);
//#endif
}

void FileManagerApp::initView()
{
    m_windowManager = new WindowManager;
}


void FileManagerApp::initController()
{
    m_appController = new AppController(this);
    m_appController->createGVfSManager();
    m_appController->createUserShareManager();
}

void FileManagerApp::initGtk()
{
    gtk_init(NULL, NULL);
    gdk_error_trap_push();
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

AppController *FileManagerApp::getAppController() const
{
    return m_appController;
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
