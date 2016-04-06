#include "filemanagerapp.h"
#include "../views/windowmanager.h"
#include "../views/dfilemanagerwindow.h"
#include "../controllers/appcontroller.h"
#include "../app/define.h"
#include "../app/global.h"
#include "dialogs/dtaskdialog.h"
#include "filesignalmanager.h"

#include <QtCore/QCoreApplication>

FileManagerApp::FileManagerApp(QObject *parent) : QObject(parent)
{
    initLogger();
    initApp();
    initView();
    initTaskDialog();
    initController();
    initCommandline();
    initGtk();
    initConnect();

    for(int i=0; i<10; i++){
        QMap<QString, QString> jobDetail;
        jobDetail.insert("jobPath", QString("%1").arg(i));
        jobDetail.insert("type", "delete");
        emit fileSignalManager->jobAdded(jobDetail);
    }
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

void FileManagerApp::initTaskDialog()
{
    m_taskDialog = new DTaskDialog;
    m_timer = new QTimer;
    m_timer->setInterval(1000);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(handleDataUpdated()));
    m_timer->start();
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
    connect(fileSignalManager, &FileSignalManager::jobAdded, m_taskDialog, &DTaskDialog::addTask);
    connect(fileSignalManager, &FileSignalManager::jobRemoved, m_taskDialog, &DTaskDialog::removeTask);
    connect(fileSignalManager, &FileSignalManager::jobDataUpdated, m_taskDialog, &DTaskDialog::handleUpdateTaskWidget);
    connect(m_taskDialog, &DTaskDialog::abortTask, fileSignalManager, &FileSignalManager::abortTask);
}

AppController *FileManagerApp::getAppController() const
{
    return m_appController;
}

void FileManagerApp::show()
{
    m_windowManager->showNewWindow();
}

void FileManagerApp::handleDataUpdated()
{

    for(int i=0; i<10; i++){

        QMap<QString, QString> jobDetail;
        jobDetail.insert("jobPath", QString("%1").arg(i));
        jobDetail.insert("type", "copy");

        QMap<QString, QString> jobDataDetail;
        jobDataDetail.insert("speed", "1M/s");
        jobDataDetail.insert("remainTime", QString("%1 s").arg(QString::number(10)));
        jobDataDetail.insert("file", "111111111111");
        jobDataDetail.insert("progress", "20");
        jobDataDetail.insert("destination", "home");
        emit fileSignalManager->jobDataUpdated(jobDetail, jobDataDetail);
    }
}


