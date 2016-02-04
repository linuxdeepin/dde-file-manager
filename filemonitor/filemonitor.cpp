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
    if (isAppGroup(path)){
        m_appGroupPath = path;
        m_delayAppGroupUpdatedTimer->start();
    }

    bool flag1 = isAppGroup(QFileInfo(path).path());
    bool flag2 = isInDesktop(path);
     if (flag1 || flag2){
         if (!isGoutputstreamTempFile(path)){
             if (flag1){
                 if (isDesktopAppFile(path)){
                     qDebug() << "create app group desktop file:" << path;
                     emit fileCreated(path);
                 }
             }
             if (flag2){
                qDebug() << "create desktop file/folder:" << path;
                emit fileCreated(path);
             }
         }
     }
}


void FileMonitor::handleMoveFrom(int cookie, QString path){
    m_moveEvent.insert(cookie, path);
    m_delayMoveOutTimer->start();
}


void FileMonitor::handleMoveTo(int cookie, QString path){
    if (isAppGroup(QFileInfo(path).path()) || isInDesktop(path)){
        if (m_moveEvent.contains(cookie)){
            QString oldPath = m_moveEvent.value(cookie);
            if (!isGoutputstreamTempFile(oldPath)){
                if (QFileInfo(oldPath).path() == QFileInfo(path).path()){
                    qDebug() << "rename file from:" << oldPath << "to" << path;
                    emit fileRenamed(oldPath, path);
                }else{
                    bool flag1 = isAppGroup(QFileInfo(oldPath).path());
                    bool flag2 = isInDesktop(oldPath);

                    bool flag3 = isAppGroup(QFileInfo(path).path());
                    bool flag4 = isInDesktop(path);

                    if (flag1){
                         if (isDesktopAppFile(oldPath)){
                             qDebug() << "move app group desktop app file out2:" << oldPath;
                             emit fileMovedOut(oldPath);
                         }
                    }
                    if (flag2){
                        qDebug() << "move desktop file/folder out2:" << oldPath;
                        emit fileMovedOut(oldPath);
                    }


                    if (flag3){
                        if (isDesktopAppFile(path)){
                            qDebug() << "move in2 app group desktop app file:" << path;
                            emit fileMovedIn(path);
                        }
                    }
                    if (flag4){
                        qDebug() << "move in2 desktop file/folder :" << path;
                        emit fileMovedIn(path);
                    }
                }
                m_moveEvent.remove(cookie);
                m_delayMoveOutTimer->start();
            }
        }else{
            qDebug() << "move in file/folder:" << path;
            emit fileMovedIn(path);
        }
    }
}

void FileMonitor::handleDelete(int cookie, QString path){
    Q_UNUSED(cookie)
    if (isAppGroup(QFileInfo(path).path()) || isInDesktop(path)){
        qDebug() << "delete" << path;
        emit fileDeleted(path);
    }
}

void FileMonitor::handleMetaDataChanged(int cookie, QString path)
{
    Q_UNUSED(cookie)
    emit fileMetaDataChanged(path);
}

void FileMonitor::delayHanleMoveFrom(){
    foreach (int cookie, m_moveEvent.keys()) {
        QString path = m_moveEvent.value(cookie);

        bool flag1 = isAppGroup(QFileInfo(path).path());
        bool flag2 =  isInDesktop(path);

        if (flag1 || flag2){
            if (!isGoutputstreamTempFile(path)){
                if (flag1){
                    if (isDesktopAppFile(path)){
                        qDebug() << "move app group desktop app file out:" << path;
                        emit fileMovedOut(path);
                    }
                }
                if (flag2){
                    qDebug() << "move desktop file/folder out:" << path;
                    emit fileMovedOut(path);
                }
            }
            m_moveEvent.remove(cookie);
        }
    }
}

void FileMonitor::delayHandleAppGroupCreated(){
    qDebug() << "app group updated:" << m_appGroupPath;
    emit appGroupUpdated(m_appGroupPath);
}


FileMonitor::~FileMonitor()
{

}

