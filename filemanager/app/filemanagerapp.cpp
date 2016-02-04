#include "filemanagerapp.h"
#include <QtCore/QCoreApplication>
#include "../views/filemanagerwindow.h"
#include "../app/define.h"
#include "../app/global.h"


FileManagerApp::FileManagerApp(QObject *parent) : QObject(parent)
{
    initApp();
    initView();
    initController();
    initLogger();
    initCommandline();
    initGtk();
    initConnect();
}

FileManagerApp::~FileManagerApp()
{

}

void FileManagerApp::initApp()
{
    qApp->setOrganizationName("deepin");
    qApp->setApplicationName(tr("dde-file-manager"));
    qApp->setApplicationVersion("2016-1.0");
}

void FileManagerApp::initView()
{
    m_fileManagerWindow = new FileManagerWindow;
}

void FileManagerApp::initController()
{

}

void FileManagerApp::initLogger()
{
    RegisterLogger();
}

void FileManagerApp::initCommandline()
{
    CommandLineManager::instance()->initOptions();
}

void FileManagerApp::initGtk()
{
    gtk_init(NULL, NULL);
    gdk_error_trap_push();
    initGtkThemeWatcher();
}

void FileManagerApp::initConnect()
{

}

void FileManagerApp::show()
{
    m_fileManagerWindow->show();
}

