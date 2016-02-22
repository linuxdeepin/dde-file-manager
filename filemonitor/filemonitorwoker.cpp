#include "filemonitorwoker.h"
#include "utils/utils.h"
#include "widgets/util.h"

FileMonitorWoker::FileMonitorWoker(QObject *parent) : QObject(parent)
{
    connect(this, SIGNAL(monitorFolderChanged(QString)), this, SLOT(monitor(QString)));
}

FileMonitorWoker::~FileMonitorWoker()
{

}

void FileMonitorWoker::addWatchFolder(const QString &path){
    int wd = inotify_add_watch(m_fd, path.toStdString().c_str(), IN_ALL_EVENTS);
    if (wd >= 0){
        if (!m_path_wd.contains(path)){
            m_path_wd.insert(path, wd);
            m_wd_path.insert(wd, path);
        }else{
            qDebug() <<"already watch for" << QString(path);
        }
    }else{
        qDebug() <<"Can't add watch for" << QString(path);
    }
}

void FileMonitorWoker::monitor(const QString &path){
    char buffer[1024];
    char* offset = NULL;
    struct inotify_event * event;
    ssize_t numRead;

    m_fd = inotify_init();
    if (m_fd < 0) {
        qDebug() << "Fail to initialize inotify";
        return;
    }

    addWatchFolder(path);

    while (true) {
        numRead = read(m_fd, buffer, MAX_BUF_SIZE);
        if (numRead == 0 || numRead == -1) {
          fprintf(stderr, "Failed to read from inotify file descriptor\n");
          exit(EXIT_FAILURE);
        }

        for (offset = buffer; offset < buffer + numRead; ) {
          event = (struct inotify_event*) offset;
          handleInotifyEvent(event);
          offset += sizeof(struct inotify_event) + event->len;
        }
   }
}


void FileMonitorWoker::handleInotifyEvent(inotify_event *event){
    QString path = joinPath(m_wd_path.value(event->wd), event->name);
    if (event->mask & IN_CREATE) {
//        qDebug() << "IN_CREATE" << path;
        emit fileCreated(event->cookie, path);
        addWatchFolder(path);
    }else if (event->mask & IN_MOVED_FROM) {
//        qDebug() << "IN_MOVED_FROM" << path;
        emit fileMovedFrom(event->cookie, path);
        unMonitor(path);
    }else if (event->mask & IN_MOVED_TO) {
//        qDebug() << "IN_MOVED_TO" << path;
        emit fileMovedTo(event->cookie, path);
        addWatchFolder(path);
    }else if (event->mask & IN_DELETE) {
//        qDebug() << "IN_DELETE" << path;
        emit fileDeleted(event->cookie, path);
        unMonitor(path);
    }else if (event->mask & IN_ATTRIB){
//        qDebug() << event->mask << path;
        emit metaDataChanged(event->cookie, path);
    }
}

void FileMonitorWoker::unMonitor(const QString &path){
    if (m_path_wd.contains(path)){
        int wd = m_path_wd.value(path);
        inotify_rm_watch(m_fd, wd);
        m_wd_path.remove(wd);
        m_path_wd.remove(path);
//        qDebug() << "unMonitor:" << path;
    }
}
