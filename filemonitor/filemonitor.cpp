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
    m_fileThread = new QThread(this);
    m_fileMonitorWorker->moveToThread(m_fileThread);
    m_fileThread->start();
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

void FileMonitor::addMonitorPath(const QString &path)
{
    if(!m_pathMonitorConuter.contains(path)) {
        QMetaObject::invokeMethod(m_fileMonitorWorker, "monitor",
                                  Qt::QueuedConnection,
                                  Q_ARG(QString, path));
    }

    m_pathMonitorConuter[path] = m_pathMonitorConuter.value(path, 0) + 1;
}

void FileMonitor::removeMonitorPath(const QString &path)
{
    if(m_pathMonitorConuter.contains(path)) {
        int count = m_pathMonitorConuter.value(path);

        --count;

        if(count == 0) {
            QMetaObject::invokeMethod(m_fileMonitorWorker, "unMonitor",
                                      Qt::QueuedConnection,
                                      Q_ARG(QString, path));

            m_pathMonitorConuter.remove(path);
        } else {
            m_pathMonitorConuter[path] = count;
        }
    }
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
    m_fileThread->terminate();
    m_fileThread->wait();
    m_fileThread->quit();
    m_fileMonitorWorker->deleteLater();
}

