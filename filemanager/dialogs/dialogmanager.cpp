#include "dialogmanager.h"
#include <QTimer>
#include "dialogs/dtaskdialog.h"
#include "../app/global.h"
#include "../app/filesignalmanager.h"
#include "../controllers/filejob.h"
#include "dialogs/messagewrongdialog.h"
#include "../dialogs/propertydialog.h"
#include "../dialogs/openwithdialog.h"
#include "../models/fileinfo.h"
#include <ddialog.h>

DWIDGET_USE_NAMESPACE

DialogManager::DialogManager(QObject *parent) : QObject(parent)
{
    initTaskDialog();
    initConnect();
}

DialogManager::~DialogManager()
{

}

void DialogManager::initTaskDialog()
{
    m_taskDialog = new DTaskDialog;
}

void DialogManager::initConnect()
{
    connect(fileSignalManager, &FileSignalManager::jobAdded, m_taskDialog, &DTaskDialog::addTask);
    connect(fileSignalManager, &FileSignalManager::jobRemoved, m_taskDialog, &DTaskDialog::removeTask);
    connect(fileSignalManager, &FileSignalManager::jobDataUpdated, m_taskDialog, &DTaskDialog::handleUpdateTaskWidget);
    connect(m_taskDialog, &DTaskDialog::abortTask, fileSignalManager, &FileSignalManager::abortTask);

    connect(fileSignalManager, &FileSignalManager::conflictDialogShowed, m_taskDialog, &DTaskDialog::showConflictDiloagByJob);
    connect(m_taskDialog, &DTaskDialog::conflictShowed, fileSignalManager, &FileSignalManager::conflictTimerStoped);
    connect(m_taskDialog, &DTaskDialog::conflictHided, fileSignalManager, &FileSignalManager::conflictTimerReStarted);
    connect(m_taskDialog, &DTaskDialog::conflictRepsonseConfirmed, fileSignalManager, &FileSignalManager::conflictRepsonseConfirmed);
    connect(m_taskDialog, &DTaskDialog::conflictRepsonseConfirmed, this, &DialogManager::handleConflictRepsonseConfirmed);

    connect(m_taskDialog, &DTaskDialog::abortTask, this, &DialogManager::abortJob);

    connect(fileSignalManager, &FileSignalManager::requestShowUrlWrongDialog, this, &DialogManager::showUrlWrongDialog);
    connect(fileSignalManager, &FileSignalManager::requestShowOpenWithDialog, this, &DialogManager::showOpenWithDialog);
    connect(fileSignalManager, &FileSignalManager::requestShowPropertyDialog, this, &DialogManager::showPropertyDialog);

}

void DialogManager::addJob(FileJob *job)
{
    m_jobs.insert(job->getJobId(), job);
}


void DialogManager::removeJob(const QString &jobId)
{
    m_jobs.remove(jobId);
}


void DialogManager::abortJob(const QMap<QString, QString> &jobDetail)
{
    QString jobId = jobDetail.value("jobId");
    FileJob * job = m_jobs.value(jobId);
    job->setApplyToAll(true);
    job->setStatus(FileJob::Cancelled);
}


void DialogManager::showUrlWrongDialog(const QString &url)
{
    MessageWrongDialog d(url);
    qDebug() << url;
    d.exec();
}

void DialogManager::showOpenWithDialog(const QString &url)
{
    qDebug() << url;
    OpenWithDialog* d = new OpenWithDialog(url);
    d->show();
}

void DialogManager::showPropertyDialog(const QString &url)
{
    FileInfo info(url);
    PropertyDialog *dialog = new PropertyDialog(&info);

    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setWindowFlags(dialog->windowFlags()
                           &~ Qt::WindowMaximizeButtonHint
                           &~ Qt::WindowMinimizeButtonHint
                           &~ Qt::WindowSystemMenuHint);
    dialog->setTitle("");
    dialog->setFixedSize(QSize(320, 480));

    dialog->show();
}


void DialogManager::handleConflictRepsonseConfirmed(const QMap<QString, QString> &jobDetail, const QMap<QString, QVariant> &response)
{
    QString jobId = jobDetail.value("jobId");
    FileJob * job = m_jobs.value(jobId);
    if(job != NULL)
    {
        bool applyToAll = response.value("applyToAll").toBool();
        int code = response.value("code").toInt();
        job->setApplyToAll(applyToAll);
        //0 = coexist, 1 = replace, 2 = skip
        switch(code)
        {
        case 0:job->setStatus(FileJob::Started);break;
        case 1:
            job->setStatus(FileJob::Started);
            job->setReplace(true);
            break;
        case 2:job->setStatus(FileJob::Cancelled);break;
        default:
            qDebug() << "unknown code"<<code;
        }
    }
}
