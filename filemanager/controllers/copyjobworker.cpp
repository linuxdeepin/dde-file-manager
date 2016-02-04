#include "copyjobworker.h"
#include "../app/global.h"
#include "dbuscontroller.h"
#include "dbusinterface/copyjob_interface.h"
#include "dbusinterface/fileoperations_interface.h"
#include "dbusinterface/services/conflictdaptor.h"
#include "../controllers/fileconflictcontroller.h"
#include "dbusinterface/services/conflictdaptor.h"
#include "widgets/util.h"

CopyjobWorker::CopyjobWorker(QStringList files, QString destination, QObject *parent) :
    QObject(parent),
    m_files(files),
    m_destination(destination)
{
    m_conflictController = new FileConflictController;
    m_progressTimer = new QTimer;
    m_progressTimer->setInterval(1000);
    m_time = new QTime;
    initConnect();
}

void CopyjobWorker::initConnect(){
    connect(this, SIGNAL(startJob()), this, SLOT(start()));
    connect(this, SIGNAL(finished()), this, SLOT(handleFinished()));
    connect(m_progressTimer, SIGNAL(timeout()), this, SLOT(handleTimeout()));
    connect(signalManager, SIGNAL(abortCopyTask(QMap<QString,QString>)),
            this, SLOT(handleTaskAborted(QMap<QString,QString>)));
}

QStringList CopyjobWorker::getFiles(){
    return m_files;
}

QString CopyjobWorker::getDestination(){
    return m_destination;
}

QString CopyjobWorker::getJobPath(){
    return m_copyjobPath;
}

const QMap<QString, QString>& CopyjobWorker::getJobDetail(){
    return m_jobDetail;
}

FileConflictController* CopyjobWorker::getFileConflictController(){
    return m_conflictController;
}

void CopyjobWorker::start(){
    copyFiles(m_files, m_destination);
}

void CopyjobWorker::copyFiles(QStringList files, QString destination){
    Q_UNUSED(destination)
    if (files.length() == 0)
        return;

    QDBusPendingReply<QString, QDBusObjectPath, QString> reply = \
            dbusController->getFileOperationsInterface()->NewCopyJob(
                files,
                desktopLocation,
                "",
                0,
                ConflictAdaptor::staticServerPath(),
                m_conflictController->getObjectPath(),
                ConflictAdaptor::staticInterfaceName()
                );

    reply.waitForFinished();
    if (!reply.isError()){
        QString service = reply.argumentAt(0).toString();
        QString path = qdbus_cast<QDBusObjectPath>(reply.argumentAt(1)).path();
        qDebug() << "copy files" << files << path;
        m_copyjobPath = path;
        m_jobDetail.insert("jobPath", path);
        m_jobDetail.insert("type", "copy");
        m_conflictController->setJobDetail(m_jobDetail);
        m_jobDataDetail.insert("destination",  QFileInfo(decodeUrl(desktopLocation)).fileName());
        m_copyJobInterface = new CopyJobInterface(service, path, QDBusConnection::sessionBus(), this);
        connectCopyJobSignal();
        m_copyJobInterface->Execute();
        m_progressTimer->start();
        m_time->start();
        emit signalManager->copyJobAdded(m_jobDetail);
    }else{
        qCritical() << reply.error().message();
        m_progressTimer->stop();
    }
}


void CopyjobWorker::connectCopyJobSignal(){
    if (m_copyJobInterface){
        connect(m_copyJobInterface, SIGNAL(Done(QString)), this, SLOT(copyJobExcuteFinished(QString)));
        connect(m_copyJobInterface, SIGNAL(Aborted()), this, SLOT(copyJobAbortFinished()));
        connect(m_copyJobInterface, SIGNAL(Copying(QString)), this, SLOT(onCopyingFile(QString)));
        connect(m_copyJobInterface, SIGNAL(TotalAmount(qlonglong,ushort)),
                this, SLOT(setTotalAmount(qlonglong,ushort)));
        connect(m_copyJobInterface, SIGNAL(ProcessedAmount(qlonglong,ushort)),
                this, SLOT(onCopyingProcessAmount(qlonglong,ushort)));
        connect(m_copyJobInterface, SIGNAL(ProcessedPercent(qlonglong)),
                this, SLOT(onProcessedPercent(qlonglong)));
    }
}


void CopyjobWorker::disconnectCopyJobSignal(){
    if (m_copyJobInterface){
        disconnect(m_copyJobInterface, SIGNAL(Done(QString)), this, SLOT(copyJobExcuteFinished(QString)));
        disconnect(m_copyJobInterface, SIGNAL(Aborted()), this, SLOT(copyJobAbortFinished()));
        disconnect(m_copyJobInterface, SIGNAL(Copying(QString)), this, SLOT(onCopyingFile(QString)));
        disconnect(m_copyJobInterface, SIGNAL(TotalAmount(qlonglong,ushort)),
                this, SLOT(setTotalAmount(qlonglong,ushort)));
        disconnect(m_copyJobInterface, SIGNAL(ProcessedAmount(qlonglong,ushort)),
                this, SLOT(onCopyingProcessAmount(qlonglong,ushort)));
        disconnect(m_copyJobInterface, SIGNAL(ProcessedPercent(qlonglong)),
                this, SLOT(onProcessedPercent(qlonglong)));
    }
}


void CopyjobWorker::copyJobExcuteFinished(QString file){
    disconnectCopyJobSignal();
    m_copyJobInterface->deleteLater();
    m_copyJobInterface = NULL;
    m_progressTimer->stop();
    emit finished();
    qDebug() << "copy job finished" << file;
}

void CopyjobWorker::copyJobAbort(){
    if (m_copyJobInterface){
        m_progressTimer->stop();
        m_copyJobInterface->Abort();
    }
}

void CopyjobWorker::copyJobAbortFinished(){
    qDebug() << "copy job aborted";
    copyJobExcuteFinished("");
}

void CopyjobWorker::onCopyingFile(QString file){
    emit signalManager->copyingFileChaned(file);
    m_jobDataDetail.insert("file", QFileInfo(decodeUrl(file)).fileName());
//    qDebug() << "onCopyingFile" << file;
    if (m_jobDetail.contains("jobPath")){
        emit signalManager->copyJobDataUpdated(m_jobDetail, m_jobDataDetail);
    }
}

void CopyjobWorker::setTotalAmount(qlonglong amount, ushort type){
    qDebug() << "========="<< amount << type;
    if (type == 0){
        m_totalAmout = amount;
    }
}

void CopyjobWorker::onCopyingProcessAmount(qlonglong progress, ushort type){
//    qDebug() << "onCopyingProcessAmount" << this << m_totalAmout << progress << type;
    if (type == 0){
        m_currentProgress = progress;
    }
}

void CopyjobWorker::onProcessedPercent(qlonglong percent){
    m_processedPercent = percent;
    m_jobDataDetail.insert("progress", QString::number(percent));
}

void CopyjobWorker::handleTimeout(){
    float speed;
    int remainTime;
    QString speedString;
    if (m_currentProgress - m_lastProgress > 1024 *1024){
        speed = (m_currentProgress - m_lastProgress) / (1024 * 1024);
        speedString = QString("%1 M/s").arg(QString::number(speed));
    }else{
        speed = (m_currentProgress - m_lastProgress) / 1024;
        speedString = QString("%1 Kb/s").arg(QString::number(speed));
    }
    if (m_currentProgress - m_lastProgress > 0){
        remainTime = (m_totalAmout - m_currentProgress) / (m_currentProgress - m_lastProgress);
    }
    m_lastProgress = m_currentProgress;

    m_jobDataDetail.insert("speed", speedString);
    m_jobDataDetail.insert("remainTime", QString("%1 s").arg(QString::number(remainTime)));
    emit signalManager->copyJobDataUpdated(m_jobDetail, m_jobDataDetail);
}

void CopyjobWorker::handleFinished(){
    qDebug() << m_jobDetail << m_jobDataDetail;
    if (m_jobDetail.contains("jobPath")){
        emit signalManager->copyJobRemoved(m_jobDetail);
    }
    m_conflictController->unRegisterDBusService();
    if (m_jobDataDetail.contains("file")){
        QString f = joinPath(desktopLocation, m_jobDataDetail.value("file"));
        if (QFile(f).exists()){
            qDebug() << f;
            emit signalManager->refreshCopyFileIcon(f);
        }
    }
}

void CopyjobWorker::handleTaskAborted(const QMap<QString, QString> &jobDetail){
    if (jobDetail == m_jobDetail){
        copyJobAbort();
    }
}

void CopyjobWorker::handleResponse(ConflictInfo obj){
    m_conflictController->getConflictAdaptor()->response(obj);
}


void CopyjobWorker::stopTimer(){
    m_progressTimer->stop();
}

void CopyjobWorker::restartTimer(){
    m_progressTimer->start();
}
