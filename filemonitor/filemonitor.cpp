#include "filemonitor.h"
#include "filemonitorwoker.h"
#include "utils.h"
#include <QDir>

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
    connect(this, SIGNAL(requestMonitorPath(QStringList)), m_fileMonitorWorker, SLOT(addPaths(QStringList)));
    connect(this, SIGNAL(requestRemoveMonitorPath(QStringList)), m_fileMonitorWorker, SLOT(removePaths(QStringList)));
}

bool FileMonitor::isGoutputstreamTempFile(QString path){
    if (QDir(path).dirName().startsWith(".goutputstream-")){
        return true;
    }
    return false;
}

void FileMonitor::addMonitorPath(const QString &path)
{
//    qDebug() << "add file monitor:" << path;

    emit requestMonitorPath(getPathParentList(path) << path);
}

void FileMonitor::removeMonitorPath(const QString &path)
{
//    qDebug() << "remove file monitor:" << path;

    emit requestRemoveMonitorPath(getPathParentList(path) << path);
}

void FileMonitor::handleCreated(int cookie, QString path){
    Q_UNUSED(cookie)
    emit fileCreated(path);
}

void FileMonitor::handleMoveFrom(int cookie, QString path){
    Q_UNUSED(cookie)
    emit fileDeleted(path);
}

void FileMonitor::handleMoveTo(int cookie, QString path){
    Q_UNUSED(cookie)
    emit fileCreated(path);
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

QStringList FileMonitor::getPathParentList(const QString &path)
{
    QStringList list;
    QDir dir(path);

    while (dir.cdUp()) {
        list << dir.absolutePath();
    }

    return list;
}

FileMonitor::~FileMonitor()
{
    m_fileThread->terminate();
    m_fileThread->wait();
    m_fileThread->quit();
    m_fileMonitorWorker->deleteLater();
}

