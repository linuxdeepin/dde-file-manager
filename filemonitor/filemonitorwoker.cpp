/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "filemonitorwoker.h"
#include "utils/utils.h"

FileMonitorWoker::FileMonitorWoker(QObject *parent) :
    QObject(parent)
{
    initInotify();
}

FileMonitorWoker::~FileMonitorWoker()
{

    m_notifier->setEnabled(false);
    foreach (int id, m_pathToID)
        inotify_rm_watch(m_inotifyFd, id < 0 ? -id : id);

    close(m_inotifyFd);

}

void FileMonitorWoker::initInotify()
{
    m_inotifyFd = -1;
#ifdef IN_CLOEXEC
    m_inotifyFd = inotify_init1(IN_CLOEXEC);
#endif
    if (m_inotifyFd == -1) {
        m_inotifyFd = inotify_init();
        if(m_inotifyFd == -1) {
            qDebug() << "Fail to initialize inotify";
            return;
        }

    }
    m_notifier = new QSocketNotifier(m_inotifyFd, QSocketNotifier::Read, this);

    fcntl(m_inotifyFd, F_SETFD, FD_CLOEXEC);
    connect(m_notifier, SIGNAL(activated(int)), this, SLOT(readFromInotify()));
}

bool FileMonitorWoker::addPath(const QString &path)
{
    qDebug() << path;
    if (path.isEmpty()) {
        qWarning("QFileSystemWatcher::addPath: path is empty");
        return true;
    }

    QStringList paths = addPaths(QStringList(path));
    return paths.isEmpty();
}

QStringList FileMonitorWoker::addPaths(const QStringList &paths)
{

    QStringList p = paths;
    QMutableListIterator<QString> it(p);

    while (it.hasNext()) {
        const QString &path = it.next();
        if (path.isEmpty())
            it.remove();
    }

    if (p.isEmpty()) {
        qWarning("QFileSystemWatcher::addPaths: list is empty");
        return QStringList();
    }


    p = addPathsAction(p);

    return p;
}


QStringList FileMonitorWoker::addPathsAction(const QStringList &paths)
{

    QStringList p = paths;
    QMutableListIterator<QString> it(p);
    while (it.hasNext()) {
        QString path = it.next();
        QFileInfo fi(path);
        bool isDir = fi.isDir();
        if (isDir) {
            if (m_directories.contains(path))
                continue;
        } else {
            if (m_files.contains(path))
                continue;
        }

        int wd = inotify_add_watch(m_inotifyFd,
                                   QFile::encodeName(path),
                                   (isDir
                                    ? (0
                                       | IN_ATTRIB
                                       | IN_MOVE
                                       | IN_CREATE
                                       | IN_DELETE
                                       | IN_DELETE_SELF
                                       )
                                    : (0
                                       | IN_ATTRIB
                                       | IN_MODIFY
                                       | IN_MOVE
                                       | IN_MOVE_SELF
                                       | IN_DELETE_SELF
                                       )));
        if (wd < 0) {
            perror("QInotifyFileSystemWatcherEngine::addPaths: inotify_add_watch failed");
            continue;
        }

        it.remove();

        int id = isDir ? -wd : wd;
        if (id < 0) {
            m_directories.append(path);
        } else {
            m_files.append(path);
        }

        m_pathToID.insert(path, id);
        m_idToPath.insert(id, path);
    }

    return p;
}

bool FileMonitorWoker::removePath(const QString &path)
{
    if (path.isEmpty()) {
        qWarning("QFileSystemWatcher::removePath: path is empty");
        return true;
    }

    QStringList paths = removePaths(QStringList(path));
    return paths.isEmpty();
}

QStringList FileMonitorWoker::removePaths(const QStringList &paths)
{


    QStringList p = paths;
    QMutableListIterator<QString> it(p);

    while (it.hasNext()) {
        const QString &path = it.next();
        if (path.isEmpty())
            it.remove();
    }

    if (p.isEmpty()) {
        qWarning("QFileSystemWatcher::removePaths: list is empty");
        return QStringList();
    }


    p = removePathsAction(p);


    return p;
}



QStringList FileMonitorWoker::removePathsAction(const QStringList &paths)
{
    QStringList p = paths;
    QMutableListIterator<QString> it(p);
    while (it.hasNext()) {
        QString path = it.next();
        int id = m_pathToID.take(path);
        QString x = m_idToPath.take(id);
        if (x.isEmpty() || x != path)
            continue;

        int wd = id < 0 ? -id : id;
        // qDebug() << "removing watch for path" << path << "wd" << wd;
        inotify_rm_watch(m_inotifyFd, wd);

        it.remove();
        if (id < 0) {
            m_directories.removeAll(path);
        } else {
            m_files.removeAll(path);
        }
    }

    return p;
}


void FileMonitorWoker::readFromInotify()
{
    int buffSize = 0;
    ioctl(m_inotifyFd, FIONREAD, (char *) &buffSize);
    QVarLengthArray<char, 4096> buffer(buffSize);
    buffSize = read(m_inotifyFd, buffer.data(), buffSize);
    char *at = buffer.data();
    char * const end = at + buffSize;

    QHash<int, inotify_event *> eventForId;
    while (at < end) {
        inotify_event *event = reinterpret_cast<inotify_event *>(at);
        handleInotifyEvent(event);
        if (eventForId.contains(event->wd))
            eventForId[event->wd]->mask |= event->mask;
        else
            eventForId.insert(event->wd, event);

        at += sizeof(inotify_event) + event->len;
    }

    QHash<int, inotify_event *>::const_iterator it = eventForId.constBegin();
    while (it != eventForId.constEnd()) {
        const inotify_event &event = **it;
        ++it;

        // qDebug() << "inotify event, wd" << event.wd << "mask" << hex << event.mask;

        int id = event.wd;
        QString path = getPathFromID(id);
        if (path.isEmpty()) {
            // perhaps a directory?
            id = -id;
            path = getPathFromID(id);
            if (path.isEmpty())
                continue;
        }

        if ((event.mask & (IN_DELETE_SELF | IN_MOVE_SELF | IN_UNMOUNT)) != 0) {
            m_pathToID.remove(path);
            m_idToPath.remove(id, getPathFromID(id));
            if (!m_idToPath.contains(id))
                inotify_rm_watch(m_inotifyFd, event.wd);

            if (id < 0)
                directoryChanged(path, true);
            else
                fileChanged(path, true);
        } else {

            if (id < 0)
                directoryChanged(path, false);
            else
                fileChanged(path, false);

        }

    }
}

void FileMonitorWoker::fileChanged(const QString &path, bool removed)
{

    if (!m_files.contains(path)) {
        // the path was removed after a change was detected, but before we delivered the signal
        return;
    }
    if (removed)
        m_files.removeAll(path);
    emit fileChanged(path);
}

void FileMonitorWoker::directoryChanged(const QString &path, bool removed)
{

    if (!m_directories.contains(path)) {
        // perhaps the path was removed after a change was detected, but before we delivered the signal
        return;
    }
    if (removed)
        m_directories.removeAll(path);
    emit directoryChanged(path);
}


QString FileMonitorWoker::getPathFromID(int id) const
{
    QHash<int, QString>::const_iterator i = m_idToPath.find(id);

    while (i != m_idToPath.constEnd() && i.key() == id) {
        if ((i + 1) == m_idToPath.constEnd() || (i + 1).key() != id) {
            return i.value();
        }
        ++i;
    }

    return QString();
}

void FileMonitorWoker::handleInotifyEvent(const inotify_event *event)
{
//    qDebug() << "counter:" <<m_counter++ << event->wd << QString::number(event->mask, 16).toUpper();
    int id = event->wd;
    QString path = getPathFromID(id);
    if (path.isEmpty()) {
        // perhaps a directory?
        id = -id;
        path = getPathFromID(id);
        if (path.isEmpty()) {
            if(id == -1)
            {
                addPath(desktopLocation);
            }
            return;

        }
    }

    path = joinPath(path, event->name);

    if(path == desktopLocation)
    {
         return;
    }
    if (event->mask & IN_CREATE) {
//        qDebug() << "IN_CREATE" << path;
        emit fileCreated(event->cookie, path);
        addPath(path);
    }else if (event->mask & IN_MOVED_FROM) {
//        qDebug() << "IN_MOVED_FROM" << path;
        emit fileMovedFrom(event->cookie, path);
        removePath(path);
    }else if (event->mask & IN_MOVED_TO) {
//        qDebug() << "IN_MOVED_TO" << path;
        emit fileMovedTo(event->cookie, path);
        addPath(path);
    }else if (event->mask & IN_DELETE) {
//        qDebug() << "IN_DELETE" << path;
        emit fileDeleted(event->cookie, path);
        removePath(path);
    }else if (event->mask & IN_ATTRIB) {
//        qDebug() << event->mask << path;
        emit metaDataChanged(event->cookie, path);
    }else{
        //qDebug() << "unknown event";
    }
}
