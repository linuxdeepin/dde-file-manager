#include "movejobworker.h"
#include "app/global.h"

MovejobWorker::MovejobWorker(QStringList files, QString destination, QObject *parent) :
    QObject(parent),
    m_files(files),
    m_destination(destination)
{
    m_progressTimer = new QTimer;
    m_progressTimer->setInterval(2000);
    m_time = new QTime;
    initConnect();
}

void MovejobWorker::initConnect(){
    connect(this, SIGNAL(startJob()), this, SLOT(start()));
    connect(this, SIGNAL(finished()), this, SLOT(handleFinished()));
    connect(m_progressTimer, SIGNAL(timeout()), this, SLOT(handleTimeout()));
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
}

void MovejobWorker::connectMoveJobSignal(){

}


void MovejobWorker::disconnectMoveJobSignal(){

}


void MovejobWorker::moveJobExcuteFinished(const QString& message){

    qDebug() << "move job finished" << message;
}

void MovejobWorker::moveJobAbort(){

}

void MovejobWorker::moveJobAbortFinished(){
    moveJobExcuteFinished("move job aborted");
}

void MovejobWorker::onMovingFile(QString file){
    m_jobDataDetail.insert("file", QFileInfo(decodeUrl(file)).fileName());
    qDebug() << "onMovingFile" << file;
    if (m_jobDetail.contains("jobPath")){

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
}

void MovejobWorker::handleFinished(){
    if (m_jobDetail.contains("jobPath")){

    }
}

void MovejobWorker::handleTaskAborted(const QMap<QString, QString> &jobDetail){
    if (jobDetail == m_jobDetail){
        moveJobAbort();
    }
}
