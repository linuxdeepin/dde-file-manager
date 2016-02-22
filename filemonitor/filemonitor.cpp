#include "filemonitor.h"
#include "filemonitorwoker.h"
#include "utils/utils.h"


FileMonitor::FileMonitor(QObject *parent) : QObject(parent)
{
    initFileMonitorWoker();
    initConnect();
}


void FileMonitor::initFileMonitorWoker(){
    m_fileMonitorWorker = new FileMonitorWoker;
    QThread* fileThread = new QThread;
    m_fileMonitorWorker->moveToThread(fileThread);
    fileThread->start();
}

void FileMonitor::initConnect(){
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

FileMonitor::~FileMonitor()
{

}

