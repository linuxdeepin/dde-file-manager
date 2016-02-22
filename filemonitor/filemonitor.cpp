#include "filemonitor.h"
#include "filemonitorwoker.h"
#include "utils/utils.h"


FileMonitor::FileMonitor(QObject *parent) : QObject(parent)
{
    initFileMonitorWoker();
    m_delayMoveOutTimer = new QTimer;
    m_delayMoveOutTimer->setInterval(100);
    m_delayMoveOutTimer->setSingleShot(true);

    m_delayAppGroupUpdatedTimer = new QTimer;
    m_delayAppGroupUpdatedTimer->setInterval(200);
    m_delayAppGroupUpdatedTimer->setSingleShot(true);

    initConnect();

    emit m_fileMonitorWorker->monitorFolderChanged(desktopLocation);
}


void FileMonitor::initFileMonitorWoker(){
    m_fileMonitorWorker = new FileMonitorWoker;
    QThread* fileThread = new QThread;
    m_fileMonitorWorker->moveToThread(fileThread);
    fileThread->start();
}

void FileMonitor::initConnect(){
    connect(m_delayMoveOutTimer, SIGNAL(timeout()), this, SLOT(delayHanleMoveFrom()));
    connect(m_delayAppGroupUpdatedTimer, SIGNAL(timeout()), this, SLOT(delayHandleAppGroupCreated()));
    connect(m_fileMonitorWorker, SIGNAL(fileCreated(int,QString)), this, SLOT(handleCreated(int,QString)));
    connect(m_fileMonitorWorker, SIGNAL(fileMovedFrom(int,QString)), this, SLOT(handleMoveFrom(int,QString)));
    connect(m_fileMonitorWorker, SIGNAL(fileMovedTo(int,QString)), this, SLOT(handleMoveTo(int,QString)));
    connect(m_fileMonitorWorker, SIGNAL(fileDeleted(int,QString)), this, SLOT(handleDelete(int,QString)));
    connect(m_fileMonitorWorker, SIGNAL(metaDataChanged(int,QString)), this, SLOT(handleMetaDataChanged(int,QString)));
}

bool FileMonitor::isGoutputstreamTempFile(QString path){
    if (QDir(path).dirName().startsWith(".goutputstream-")){
        return true;
    }
    return false;
}

void FileMonitor::handleCreated(int cookie, QString path){
    Q_UNUSED(cookie)
    emit fileCreated(path);
}


void FileMonitor::handleMoveFrom(int cookie, QString path){
    m_moveEvent.insert(cookie, path);
    m_delayMoveOutTimer->start();
}


void FileMonitor::handleMoveTo(int cookie, QString path){
    qDebug() << cookie << path;
}

void FileMonitor::handleDelete(int cookie, QString path){
    Q_UNUSED(cookie)
    emit fileDeleted(path);
}

void FileMonitor::handleMetaDataChanged(int cookie, QString path)
{
    Q_UNUSED(cookie)
    emit fileMetaDataChanged(path);
}

void FileMonitor::delayHanleMoveFrom(){
    foreach (int cookie, m_moveEvent.keys()) {
            QString path = m_moveEvent.value(cookie);
            emit fileMovedOut(path);
            m_moveEvent.remove(cookie);
    }
}

void FileMonitor::delayHandleAppGroupCreated(){
    qDebug() << "app group updated:" << m_appGroupPath;
    emit appGroupUpdated(m_appGroupPath);
}


FileMonitor::~FileMonitor()
{

}

