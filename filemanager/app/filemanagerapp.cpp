#include "filemanagerapp.h"
#include <QtCore/QCoreApplication>
#include "../views/windowmanager.h"
#include "../views/dfilemanagerwindow.h"
#include "../controllers/appcontroller.h"
#include "../app/define.h"
#include "../app/global.h"


FileManagerApp::FileManagerApp(QObject *parent) : QObject(parent)
{
    initLogger();
    initApp();
    initView();
    initController();
    initCommandline();
    initGtk();
    initConnect();
}

FileManagerApp::~FileManagerApp()
{

}

void FileManagerApp::initLogger()
{
    RegisterLogger();
}

void FileManagerApp::initGtk()
{
    gtk_init(NULL, NULL);
    gdk_error_trap_push();
    initGtkThemeWatcher();
}


void FileManagerApp::initApp()
{
    qApp->setOrganizationName("deepin");
    qApp->setApplicationName(tr("dde-file-manager"));
    qApp->setApplicationVersion("2016-1.0");
}

void FileManagerApp::initView()
{
    m_windowManager = new WindowManager;
//#if DEEPIN_UI_ENABLE
//    m_dFileMangerWindow = new DFileManagerWindow;
//#else
//    m_fileManagerWindow = new FileManagerWindow;
//#endif
}

void FileManagerApp::initController()
{
    m_appController = new AppController(this);
}

void FileManagerApp::initManager()
{

}



void FileManagerApp::initCommandline()
{
    CommandLineManager::instance()->initOptions();
}

void FileManagerApp::initTranslation()
{

}

void FileManagerApp::initConnect()
{

}

AppController *FileManagerApp::getAppController() const
{
    return m_appController;
}

void FileManagerApp::show()
{
    m_windowManager->showNewWindow();
}

