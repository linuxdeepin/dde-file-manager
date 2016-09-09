 /**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "filemonitorwoker.h"
#include "utils.h"
#include <QFileInfo>
#include <QProcess>

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

bool FileMonitorWoker::fileNameCorrection(const QString &filePath)
{
    QFileInfo info(filePath);
    QProcess ls;

    ls.start("ls", QStringList() << "-1" << "--color=never" << info.absolutePath());
    ls.waitForFinished();

    const QByteArray &request = ls.readAllStandardOutput();

    for (const QByteArray &name : request.split('\n')) {
        const QString str_fileName = QString::fromLocal8Bit(name);

        if (str_fileName == info.fileName() && str_fileName.toLocal8Bit() != name) {
            return fileNameCorrection(joinPath(info.absolutePath().toLocal8Bit(), name));
        }
    }

    return false;
}

bool FileMonitorWoker::fileNameCorrection(const QByteArray &filePath)
{
    const QByteArray &newFilePath = QString::fromLocal8Bit(filePath).toLocal8Bit();

    if (filePath == newFilePath)
        return true;

    return std::rename(filePath.constData(), newFilePath.constData());
}

QStringList FileMonitorWoker::addPathsAction(const QStringList &paths)
{

    QStringList p = paths;
    QMutableListIterator<QString> it(p);
    while (it.hasNext()) {
        QString path = it.next();
        QFileInfo fi(path);
        bool isDir = fi.isDir();

        if (m_pathToID.contains(path)) {
            m_pathReferenceCounts[path] = m_pathReferenceCounts.value(path, 0) + 1;
        }

        int wd = addWatch(path, isDir);
        if (wd < 0) {
            perror("QInotifyFileSystemWatcherEngine::addPaths: inotify_add_watch failed");
            continue;
        }

        it.remove();

        int id = isDir ? -wd : wd;

        m_pathToID.insert(path, id);
        m_idToPath.insert(id, path);
    }

    return p;
}

bool FileMonitorWoker::addPaths(const QStringList &list)
{
    if (list.isEmpty()) {
        qWarning("path list is empty");
        return true;
    }

    QStringList p = list;
    QMutableListIterator<QString> it(p);

    while (it.hasNext()) {
        const QString &path = it.next();
        if (path.isEmpty())
            it.remove();
    }

    if (p.isEmpty()) {
        qWarning("QFileSystemWatcher::addPaths: list is empty");
        return true;
    }

    p = addPathsAction(p);

    return !p.isEmpty();
}

bool FileMonitorWoker::removePaths(const QStringList &list)
{
    QStringList p = list;
    QMutableListIterator<QString> it(p);

    while (it.hasNext()) {
        const QString &path = it.next();
        if (path.isEmpty())
            it.remove();
    }

    if (p.isEmpty()) {
        qWarning("QFileSystemWatcher::removePaths: list is empty");
        return true;
    }

    p = removePathsAction(p);

    return !p.isEmpty();
}

QStringList FileMonitorWoker::removePathsAction(const QStringList &paths)
{
    QStringList p = paths;
    QMutableListIterator<QString> it(p);
    while (it.hasNext()) {
        QString path = it.next();

        int count = m_pathReferenceCounts.value(path, 0) - 1;

        if (count > 0) {
            m_pathReferenceCounts[path] = count;

            continue;
        }

        m_pathReferenceCounts.remove(path);

        rmWatch(path);

        it.remove();
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

//        if ((event.mask & (IN_DELETE_SELF | IN_MOVE_SELF | IN_UNMOUNT)) != 0) {
//            m_pathToID.remove(path);
//            m_idToPath.remove(id, getPathFromID(id));
//            if (!m_idToPath.contains(id))
//                inotify_rm_watch(m_inotifyFd, event.wd);

//            if (id < 0)
//                directoryChanged(path, true);
//            else
//                fileChanged(path, true);
//        } else {

            if (id < 0)
                directoryChanged(path);
            else
                fileChanged(path);

//        }

    }
}

//void FileMonitorWoker::fileChanged(const QString &path, bool removed)
//{

//    if (!m_files.contains(path)) {
//        // the path was removed after a change was detected, but before we delivered the signal
//        return;
//    }
//    if (removed)
//        m_files.removeAll(path);
//    emit fileChanged(path);
//}

//void FileMonitorWoker::directoryChanged(const QString &path, bool removed)
//{

//    if (!m_directories.contains(path)) {
//        // perhaps the path was removed after a change was detected, but before we delivered the signal
//        return;
//    }
//    if (removed)
//        m_directories.removeAll(path);
//    emit directoryChanged(path);
//}


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
//    qDebug() << event->wd << QString::number(event->mask, 16).toUpper();
    int id = event->wd;
    QByteArray path = getPathFromID(id).toLocal8Bit();
    if (path.isEmpty()) {
        // perhaps a directory?
        id = -id;
        path = getPathFromID(id).toLocal8Bit();
        if (path.isEmpty()) {
            return;
        }
    }

    path = joinPath(path, event->name);

    if (event->name != QString::fromLocal8Bit(event->name).toLocal8Bit()) {
        if (event->mask & (IN_CREATE | IN_MOVED_TO)) {
            fileNameCorrection(path);
        }
    }

    if (event->mask & IN_CREATE) {
        qDebug() << "IN_CREATE" << path;

        emit fileCreated(event->cookie, path);

        if (m_pathToID.contains(path)) {
            addWatch(path, QFileInfo(path).isDir());
        }

        for (const QString &tmp_path : m_pathToID.keys()) {
            if (tmp_path == path)
                continue;

            if (tmp_path.startsWith(path) && QFile::exists(tmp_path))
                emit fileCreated(event->cookie, tmp_path);
        }
    }

    if (event->mask & IN_MOVED_FROM) {
        qDebug() << "IN_MOVED_FROM" << path;
        emit fileMovedFrom(event->cookie, path);

        for (const QString &tmp_path : m_pathToID.keys()) {
            if (tmp_path == path)
                continue;

            if (tmp_path.startsWith(path))
                emit fileDeleted(event->cookie, tmp_path);
        }
    }

    if (event->mask & IN_MOVED_TO) {
        qDebug() << "IN_MOVED_TO" << path;
        emit fileMovedTo(event->cookie, path);

        if (m_pathToID.contains(path)) {
            addWatch(path, QFileInfo(path).isDir());
        }

        if (!(event->mask & IN_CREATE)) {
            for (const QString &tmp_path : m_pathToID.keys()) {
                if (tmp_path == path)
                    continue;

                if (tmp_path.startsWith(path) && QFile::exists(tmp_path))
                    emit fileCreated(event->cookie, tmp_path);
            }
        }
    }

    if (event->mask & IN_DELETE) {
        qDebug() << "IN_DELETE" << path;

        emit fileDeleted(event->cookie, path);
    }

    if (event->mask & IN_ATTRIB) {
//        qDebug() << event->mask << path;
        emit metaDataChanged(event->cookie, path);
    }/*else if (event->mask & IN_MOVE_SELF){
        qDebug() << "IN_MOVE_SELF" << path;
        emit fileDeleted(event->cookie, path);
    }*/
}

int FileMonitorWoker::addWatch(const QString &path, bool isDir)
{
    int wd = inotify_add_watch(m_inotifyFd,
                             QFile::encodeName(path),
                             (isDir
                              ? (0
                                 | IN_ATTRIB
                                 | IN_MOVE
                                 | IN_CREATE
                                 | IN_DELETE
                                 | IN_DELETE_SELF
                                 | IN_MOVE_SELF
                                 )
                              : (0
                                 | IN_ATTRIB
                                 | IN_MODIFY
                                 | IN_MOVE
                                 | IN_MOVE_SELF
                                 | IN_DELETE_SELF
                                 | IN_MOVE_SELF)));

    if (wd >= 0) {
        m_idToPath.remove(m_pathToID.value(path));
        m_pathToID[path] = wd;
        m_idToPath.insert(wd, path);
    }

    return wd;
}

int FileMonitorWoker::rmWatch(const QString &path)
{
    int id = m_pathToID.take(path);
    int wd = inotify_rm_watch(m_inotifyFd, qAbs(id));

    m_idToPath.remove(id);

    return wd;
}
