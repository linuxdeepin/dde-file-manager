#include "filemanagerapp.h"
#include "../views/windowmanager.h"
#include "../views/dfilemanagerwindow.h"
#include "../controllers/appcontroller.h"
#include "../app/define.h"
#include "../app/global.h"
#include "filesignalmanager.h"
#include "../dialogs/dialogmanager.h"


#include <QtCore/QCoreApplication>

FileManagerApp::FileManagerApp(QObject *parent) : QObject(parent)
{
    initLogger();
    initApp();
    initView();
    initDialogManager();
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
}

void FileManagerApp::initDialogManager()
{
    //m_dialogManager = new DialogManager;
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


