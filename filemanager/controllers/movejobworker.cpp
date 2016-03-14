#include "movejobworker.h"
#include "dbusinterface/movejob_interface.h"
#include "dbusinterface/fileoperations_interface.h"
#include "dbusinterface/services/conflictdaptor.h"
#include "../app/global.h"
#include "../controllers/fileconflictcontroller.h"

MovejobWorker::MovejobWorker(QStringList files, QString destination, QObject *parent) :
    QObject(parent),
    m_files(files),
    m_destination(destination)
{
    m_conflictController = new FileConflictController;
    m_progressTimer = new QTimer;
    m_progressTimer->setInterval(2000);
    m_time = new QTime;
    initConnect();
}

void MovejobWorker::initConnect(){
    connect(this, SIGNAL(startJob()), this, SLOT(start()));
    connect(this, SIGNAL(finished()), this, SLOT(handleFinished()));
    connect(m_progressTimer, SIGNAL(timeout()), this, SLOT(handleTimeout()));
    connect(signalManager, SIGNAL(abortMoveTask(QMap<QString,QString>)),
            this, SLOT(handleTaskAborted(QMap<QString,QString>)));
}

QStringList MovejobWorker::getFiles(){
    return m_files;
}

QString MovejobWorker::getDestination(){
    return m_destination;
}

QString MovejobWorker::getJobPath(){
    return m_movejobPath;
}

const QMap<QString, QString>& MovejobWorker::getJobDetail(){
    return m_jobDetail;
}

FileConflictController* MovejobWorker::getFileConflictController(){
    return m_conflictController;
}


void MovejobWorker::start(){
    moveFiles(m_files, m_destination);
}

void MovejobWorker::moveFiles(QStringList files, QString destination){
    qDebug() << files << destination;

//    if (files.length() == 0)
//        return;

//    QDBusPendingReply<QString, QDBusObjectPath, QString> reply = \
//            dbusController->getFileOperationsInterface()->NewMoveJob(
//                files,
//                destination,
//                "",
//                0,
//                ConflictAdaptor::staticServerPath(),
//                m_conflictController->getObjectPath(),
//                ConflictAdaptor::staticInterfaceName()
//                );

////    QDBusPendingReply<QString, QDBusObjectPath, QString> reply = dbusController->getFileOperationsInterface()->NewMoveJob(files, destination, "",  0, "",  "", "");
//    reply.waitForFinished();
//    if (!reply.isError()){
//        QString service = reply.argumentAt(0).toString();
//        QString path = qdbus_cast<QDBusObjectPath>(reply.argumentAt(1)).path();
//        qDebug() << "move files" << files << path;
//        m_movejobPath = path;
//        m_jobDetail.insert("jobPath", path);
//        m_jobDetail.insert("type", "move");
//        m_conflictController->setJobDetail(m_jobDetail);
//        m_jobDataDetail.insert("destination",  QFileInfo(decodeUrl(desktopLocation)).fileName());
//        m_moveJobInterface = new MoveJobInterface(service, path, QDBusConnection::sessionBus(), this);
//        connectMoveJobSignal();
//        m_moveJobInterface->Execute();
//        m_progressTimer->start();
//        m_time->start();
//        emit signalManager->moveJobAdded(m_jobDetail);
//    }else{
//        qCritical() << reply.error().message();
//        m_progressTimer->stop();
//    }
}

void MovejobWorker::connectMoveJobSignal(){
    if (m_moveJobInterface){
        connect(m_moveJobInterface, SIGNAL(Done(QString)), this, SLOT(moveJobExcuteFinished(QString)));
        connect(m_moveJobInterface, SIGNAL(Aborted()), this, SLOT(moveJobAbortFinished()));
        connect(m_moveJobInterface, SIGNAL(Moving(QString)), this, SLOT(onMovingFile(QString)));
        connect(m_moveJobInterface, SIGNAL(TotalAmount(qlonglong,ushort)),
                this, SLOT(setTotalAmount(qlonglong,ushort)));
        connect(m_moveJobInterface, SIGNAL(ProcessedAmount(qlonglong,ushort)),
                this, SLOT(onMovingProcessAmount(qlonglong,ushort)));
    }
}


void MovejobWorker::disconnectMoveJobSignal(){
    if (m_moveJobInterface){
        disconnect(m_moveJobInterface, SIGNAL(Done(QString)), this, SLOT(moveJobExcuteFinished(QString)));
        disconnect(m_moveJobInterface, SIGNAL(Aborted()), this, SLOT(moveJobAbortFinished()));
        disconnect(m_moveJobInterface, SIGNAL(Moving(QString)), this, SLOT(onMovingFile(QString)));
        disconnect(m_moveJobInterface, SIGNAL(TotalAmount(qlonglong,ushort)),
                this, SLOT(setTotalAmount(qlonglong,ushort)));
        disconnect(m_moveJobInterface, SIGNAL(ProcessedAmount(qlonglong,ushort)),
                this, SLOT(onMovingProcessAmount(qlonglong,ushort)));
    }
}


void MovejobWorker::moveJobExcuteFinished(const QString& message){
    disconnectMoveJobSignal();
    m_moveJobInterface->deleteLater();
    m_moveJobInterface = NULL;
    m_progressTimer->stop();
    emit finished();
    qDebug() << "move job finished" << message;
}

void MovejobWorker::moveJobAbort(){
    if (m_moveJobInterface){
        m_progressTimer->stop();
        m_moveJobInterface->Abort();
    }
}

void MovejobWorker::moveJobAbortFinished(){
    moveJobExcuteFinished("move job aborted");
}

void MovejobWorker::onMovingFile(QString file){
    emit signalManager->movingFileChaned(file);
    m_jobDataDetail.insert("file", QFileInfo(decodeUrl(file)).fileName());
    qDebug() << "onMovingFile" << file;
    if (m_jobDetail.contains("jobPath")){
        emit signalManager->moveJobDataUpdated(m_jobDetail, m_jobDataDetail);
    }
}

void MovejobWorker::setTotalAmount(qlonglong amount, ushort type){
    qDebug() << "========="<< amount << type;
    if (type == 0){
        m_totalAmout = amount;
    }
}

void MovejobWorker::onMovingProcessAmount(qlonglong progress, ushort info){
    if (info == 0){
        m_currentProgress = progress;
    }
    qDebug() << "onMovingProcessAmount" << progress << info;
}

void MovejobWorker::handleTimeout(){
    float speed = (m_currentProgress - m_lastProgress) / (1024 * 1024);
//    qDebug() << speed;
    m_lastProgress = m_currentProgress;
    int remainTime = (m_totalAmout - m_currentProgress) / speed;
//    qDebug() << remainTime;
    m_jobDataDetail.insert("speed", QString::number(speed));
    m_jobDataDetail.insert("remainTime", QString::number(remainTime));
    emit signalManager->moveJobDataUpdated(m_jobDetail, m_jobDataDetail);
}

void MovejobWorker::handleFinished(){
    if (m_jobDetail.contains("jobPath")){
        emit signalManager->moveJobRemoved(m_jobDetail);
    }
    m_conflictController->unRegisterDBusService();
}

void MovejobWorker::handleTaskAborted(const QMap<QString, QString> &jobDetail){
    if (jobDetail == m_jobDetail){
        moveJobAbort();
    }
}

void MovejobWorker::handleResponse(ConflictInfo obj){
    m_conflictController->getConflictAdaptor()->response(obj);
}

