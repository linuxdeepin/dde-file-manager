#include "dialogmanager.h"
#include <QTimer>
#include "dialogs/dtaskdialog.h"
#include "../app/global.h"
#include "../app/filesignalmanager.h"


DialogManager::DialogManager(QObject *parent) : QObject(parent)
{
    initTaskDialog();
    initConnect();
    handleDataUpdated();
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
}

void DialogManager::handleDataUpdated()
{
    connect(m_taskDialog, &DTaskDialog::conflictRepsonseConfirmed, this, &DialogManager::handleConflictRepsonseConfirmed);
    for(int i=0; i<10; i++){
        QMap<QString, QString> jobDetail;
        jobDetail.insert("jobId", QString("%1").arg(i));
        jobDetail.insert("type", "copy");
        emit fileSignalManager->jobAdded(jobDetail);

        QMap<QString, QString> jobDataDetail;
        jobDataDetail.insert("speed", "1M/s");
        jobDataDetail.insert("remainTime", QString("%1 s").arg(QString::number(10)));
        jobDataDetail.insert("file", "111111111111");
        jobDataDetail.insert("progress", "20");
        jobDataDetail.insert("destination", "home");
        emit fileSignalManager->jobDataUpdated(jobDetail, jobDataDetail);
        emit fileSignalManager->conflictDialogShowed(jobDetail);
    }

}

void DialogManager::handleConflictRepsonseConfirmed(const QMap<QString, QString> &jobDetail, const QMap<QString, QVariant> &response)
{
    qDebug() << jobDetail << response;
}
