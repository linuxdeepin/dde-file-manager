/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "dfilesystemwatcher.h"
#include "private/dfilesystemwatcher_p.h"
#include "dfmglobal.h"

#include <QFileInfo>
#include <QDir>
#include <QDebug>

#if defined(Q_OS_LINUX)
#include <sys/inotify.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#endif

DFileSystemWatcherPrivate::DFileSystemWatcherPrivate(int fd, DFileSystemWatcher *qq)
    : q_ptr(qq)
    , inotifyFd(fd)
    , notifier(fd, QSocketNotifier::Read, qq)
{
    fcntl(inotifyFd, F_SETFD, FD_CLOEXEC);
    qq->connect(&notifier, SIGNAL(activated(int)), q_ptr, SLOT(_q_readFromInotify()));
}

DFileSystemWatcherPrivate::~DFileSystemWatcherPrivate()
{
    notifier.setEnabled(false);
    foreach (int id, pathToID)
        inotify_rm_watch(inotifyFd, id < 0 ? -id : id);

    ::close(inotifyFd);
}

QStringList DFileSystemWatcherPrivate::addPaths(const QStringList &paths, QStringList *files, QStringList *directories)
{
    QStringList p = paths;
    QMutableListIterator<QString> it(p);
    while (it.hasNext()) {
        QString path = it.next();
        QFileInfo fi(path);
        bool isDir = fi.isDir();
        if (isDir) {
            if (directories->contains(path))
                continue;
        } else {
            if (files->contains(path))
                continue;
        }

        int wd = inotify_add_watch(inotifyFd,
                                   QFile::encodeName(path),
                                   (isDir
                                    ? (0
                                       | IN_ATTRIB
                                       | IN_MOVE
                                       | IN_MOVE_SELF
                                       | IN_CREATE
                                       | IN_DELETE
                                       | IN_DELETE_SELF
                                       | IN_MODIFY
                                       )
                                    : (0
                                       | IN_ATTRIB
                                       | IN_CLOSE_WRITE
                                       | IN_MODIFY
                                       | IN_MOVE
                                       | IN_MOVE_SELF
                                       | IN_DELETE_SELF
                                       )));
        if (wd < 0) {
            perror("DFileSystemWatcherPrivate::addPaths: inotify_add_watch failed");
            continue;
        }

        it.remove();

        int id = isDir ? -wd : wd;
        if (id < 0) {
            directories->append(path);
        } else {
            files->append(path);
        }

        pathToID.insert(path, id);
        idToPath.insert(id, path);
    }

    return p;
}

QStringList DFileSystemWatcherPrivate::removePaths(const QStringList &paths, QStringList *files, QStringList *directories)
{
    QStringList p = paths;
    QMutableListIterator<QString> it(p);
    while (it.hasNext()) {
        QString path = it.next();
        int id = pathToID.take(path);
        QString x = idToPath.take(id);

        it.remove();

        if (x.isEmpty() || x != path)
            continue;

        int wd = id < 0 ? -id : id;
        // qDebug() << "removing watch for path" << path << "wd" << wd;
        inotify_rm_watch(inotifyFd, wd);

        if (id < 0) {
            directories->removeAll(path);
        } else {
            files->removeAll(path);
        }
    }

    return p;
}

void DFileSystemWatcherPrivate::_q_readFromInotify()
{
    Q_Q(DFileSystemWatcher);
//    qDebug() << "QInotifyFileSystemWatcherEngine::readFromInotify";

    int buffSize = 0;
    ioctl(inotifyFd, FIONREAD, (char *) &buffSize);
    QVarLengthArray<char, 4096> buffer(buffSize);
    buffSize = read(inotifyFd, buffer.data(), buffSize);
    char *at = buffer.data();
    char * const end = at + buffSize;

    QList<inotify_event *> eventList;
    QHash<int, QString> pathForId;
    /// only save event: IN_MOVE_TO
    QMap<int, QString> cookieToFilePath;
    QMap<int, QString> cookieToFileName;
    QSet<int> hasMoveFromByCookie;
    while (at < end) {
        inotify_event *event = reinterpret_cast<inotify_event *>(at);
        QString path;

        at += sizeof(inotify_event) + event->len;

        int id = event->wd;
        path = getPathFromID(id);
        if (path.isEmpty()) {
            // perhaps a directory?
            id = -id;
            path = getPathFromID(id);
            if (path.isEmpty())
                continue;
        }

        if (!(event->mask & IN_MOVED_TO) || !hasMoveFromByCookie.contains(event->cookie)) {
            eventList.append(event);
            pathForId.insert(id, path);
        }

        if (event->mask & IN_MOVED_TO) {
            cookieToFilePath.insert(event->cookie, path);
            cookieToFileName.insert(event->cookie, QString::fromUtf8(event->name));
        }

        if (event->mask & IN_MOVED_FROM)
            hasMoveFromByCookie << event->cookie;
    }

//    qDebug() << "event count:" << eventList.count();

    QList<inotify_event *>::const_iterator it = eventList.constBegin();
    while (it != eventList.constEnd()) {
        const inotify_event &event = **it;
        ++it;

//        qDebug() << "inotify event, wd" << event.wd << "cookie" << event.cookie << "mask" << hex << event.mask;

        int id = event.wd;
        QString path = pathForId.value(id);

        if (path.isEmpty()) {
            id = -id;
            path = pathForId.value(id);

            if (path.isEmpty())
                continue;
        }
        const QString &name = QString::fromUtf8(event.name);

//        qDebug() << "event for path" << path;

        /// TODO: Existence of invalid utf8 characters QFile can not read the file information
        if (event.name != QString::fromLocal8Bit(event.name).toLocal8Bit()) {
            if (event.mask & (IN_CREATE | IN_MOVED_TO)) {
                DFMGlobal::fileNameCorrection(path);
            }
        }

        if ((event.mask & (IN_DELETE_SELF | IN_MOVE_SELF | IN_UNMOUNT)) != 0) {
            do {
                if (event.mask & IN_MOVE_SELF) {
                    QMap<int, QString>::const_iterator iterator = cookieToFilePath.constBegin();

                    bool isMove = false;

                    while (iterator != cookieToFilePath.constEnd()) {
                        const QString &_path = iterator.value();
                        const QString &_name = cookieToFileName.value(iterator.key());

                        if (QFileInfo(_path + QDir::separator() + _name) == QFileInfo(path)) {
                            isMove = true;
                            break;
                        }

                        ++iterator;
                    }

                    if (isMove)
                        break;
                }

                /// Keep watcher
//                pathToID.remove(path);
//                idToPath.remove(id, getPathFromID(id));
//                if (!idToPath.contains(id))
//                    inotify_rm_watch(inotifyFd, event.wd);

//                if (id < 0)
//                    onDirectoryChanged(path, true);
//                else
//                    onFileChanged(path, true);

                emit q->fileDeleted(path, QString(), DFileSystemWatcher::QPrivateSignal());
            } while (false);
        } else {
            if (id < 0)
                onDirectoryChanged(path, false);
            else
                onFileChanged(path, false);
        }

        QString filePath = path;

        if (id < 0) {
            if (path.endsWith(QDir::separator()))
                filePath = path + name;
            else
                filePath = path  + QDir::separator() + name;
        }

        if (event.mask & IN_CREATE) {
//            qDebug() << "IN_CREATE" << filePath << name;

            if (name.isEmpty()) {
                if (pathToID.contains(path)) {
                    q->removePath(path);
                    q->addPath(path);
                }
            } else if (pathToID.contains(filePath)) {
                q->removePath(filePath);
                q->addPath(filePath);
            }

            emit q->fileCreated(path, name, DFileSystemWatcher::QPrivateSignal());
        }

        if (event.mask & IN_DELETE) {
//            qDebug() << "IN_DELETE" << filePath;

            emit q->fileDeleted(path, name, DFileSystemWatcher::QPrivateSignal());
        }

        if (event.mask & IN_MOVED_FROM) {
            const QString &toPath = cookieToFilePath.value(event.cookie);
            const QString toName = cookieToFileName.value(event.cookie);

//            qDebug() << "IN_MOVED_FROM" << filePath << "to path:" << toPath << "to name:" << toName;

            emit q->fileMoved(path, name, toPath, toName, DFileSystemWatcher::QPrivateSignal());
        }

        if (event.mask & IN_MOVED_TO) {
//            qDebug() << "IN_MOVED_TO" << filePath;

            if (!hasMoveFromByCookie.contains(event.cookie))
                emit q->fileMoved(QString(), QString(), path, name, DFileSystemWatcher::QPrivateSignal());
        }

        if (event.mask & IN_ATTRIB) {
//            qDebug() << "IN_ATTRIB" <<  event.mask << filePath;

            emit q->fileAttributeChanged(path, name, DFileSystemWatcher::QPrivateSignal());
        }

        /*only monitor file close event which is opend by write mode*/
        if (event.mask & IN_CLOSE_WRITE) {
//            qDebug() << "IN_CLOSE_WRITE" <<  event.mask << filePath;

            emit q->fileClosed(path, id < 0 ? name : QString(), DFileSystemWatcher::QPrivateSignal());
        }

        if (event.mask & IN_MODIFY) {
//            qDebug() << "IN_MODIFY" <<  event.mask << filePath << name;

            emit q->fileModified(path, name, DFileSystemWatcher::QPrivateSignal());
        }
    }
}

QString DFileSystemWatcherPrivate::getPathFromID(int id) const
{
    QHash<int, QString>::const_iterator i = idToPath.find(id);
    while (i != idToPath.constEnd() && i.key() == id) {
        if ((i + 1) == idToPath.constEnd() || (i + 1).key() != id) {
            return i.value();
        }
        ++i;
    }
    return QString();
}

void DFileSystemWatcherPrivate::onFileChanged(const QString &path, bool removed)
{
    Q_Q(DFileSystemWatcher);
    if (!files.contains(path)) {
        // the path was removed after a change was detected, but before we delivered the signal
        return;
    }
    if (removed) {
        files.removeAll(path);
    }
//    emit q->fileChanged(path, DFileSystemWatcher::QPrivateSignal());
}

void DFileSystemWatcherPrivate::onDirectoryChanged(const QString &path, bool removed)
{
    Q_Q(DFileSystemWatcher);
    if (!directories.contains(path)) {
        // perhaps the path was removed after a change was detected, but before we delivered the signal
        return;
    }
    if (removed) {
        directories.removeAll(path);
    }
//    emit q->directoryChanged(path, DFileSystemWatcher::QPrivateSignal());
}

/*!
    \class DFileSystemWatcher
    \inmodule QtCore
    \brief The DFileSystemWatcher class provides an interface for monitoring files and directories for modifications.
    \ingroup io
    \since 4.2
    \reentrant

    DFileSystemWatcher monitors the file system for changes to files
    and directories by watching a list of specified paths.

    Call addPath() to watch a particular file or directory. Multiple
    paths can be added using the addPaths() function. Existing paths can
    be removed by using the removePath() and removePaths() functions.

    DFileSystemWatcher examines each path added to it. Files that have
    been added to the DFileSystemWatcher can be accessed using the
    files() function, and directories using the directories() function.

    The fileChanged() signal is emitted when a file has been modified,
    renamed or removed from disk. Similarly, the directoryChanged()
    signal is emitted when a directory or its contents is modified or
    removed.  Note that DFileSystemWatcher stops monitoring files once
    they have been renamed or removed from disk, and directories once
    they have been removed from disk.

    \note On systems running a Linux kernel without inotify support,
    file systems that contain watched paths cannot be unmounted.

    \note Windows CE does not support directory monitoring by
    default as this depends on the file system driver installed.

    \note The act of monitoring files and directories for
    modifications consumes system resources. This implies there is a
    limit to the number of files and directories your process can
    monitor simultaneously. On all BSD variants, for
    example, an open file descriptor is required for each monitored
    file. Some system limits the number of open file descriptors to 256
    by default. This means that addPath() and addPaths() will fail if
    your process tries to add more than 256 files or directories to
    the file system monitor. Also note that your process may have
    other file descriptors open in addition to the ones for files
    being monitored, and these other open descriptors also count in
    the total. OS X uses a different backend and does not
    suffer from this issue.


    \sa QFile, QDir
*/


/*!
    Constructs a new file system watcher object with the given \a parent.
*/
DFileSystemWatcher::DFileSystemWatcher(QObject *parent)
    : QObject(parent)
{
    int fd = -1;
#ifdef IN_CLOEXEC
    fd = inotify_init1(IN_CLOEXEC);
#endif
    if (fd == -1) {
        fd = inotify_init();
    }

    if (fd != -1)
        d_ptr.reset(new DFileSystemWatcherPrivate(fd, this));
}

/*!
    Constructs a new file system watcher object with the given \a parent
    which monitors the specified \a paths list.
*/
DFileSystemWatcher::DFileSystemWatcher(const QStringList &paths, QObject *parent)
    : DFileSystemWatcher(parent)
{
    addPaths(paths);
}

/*!
    Destroys the file system watcher.
*/
DFileSystemWatcher::~DFileSystemWatcher()
{ }

/*!
    Adds \a path to the file system watcher if \a path exists. The
    path is not added if it does not exist, or if it is already being
    monitored by the file system watcher.

    If \a path specifies a directory, the directoryChanged() signal
    will be emitted when \a path is modified or removed from disk;
    otherwise the fileChanged() signal is emitted when \a path is
    modified, renamed or removed.

    If the watch was successful, true is returned.

    Reasons for a watch failure are generally system-dependent, but
    may include the resource not existing, access failures, or the
    total watch count limit, if the platform has one.

    \note There may be a system dependent limit to the number of
    files and directories that can be monitored simultaneously.
    If this limit is been reached, \a path will not be monitored,
    and false is returned.

    \sa addPaths(), removePath()
*/
bool DFileSystemWatcher::addPath(const QString &path)
{
    if (path.isEmpty()) {
        qWarning("DFileSystemWatcher::addPath: path is empty");
        return true;
    }

    QStringList paths = addPaths(QStringList(path));
    return paths.isEmpty();
}

/*!
    Adds each path in \a paths to the file system watcher. Paths are
    not added if they not exist, or if they are already being
    monitored by the file system watcher.

    If a path specifies a directory, the directoryChanged() signal
    will be emitted when the path is modified or removed from disk;
    otherwise the fileChanged() signal is emitted when the path is
    modified, renamed, or removed.

    The return value is a list of paths that could not be watched.

    Reasons for a watch failure are generally system-dependent, but
    may include the resource not existing, access failures, or the
    total watch count limit, if the platform has one.

    \note There may be a system dependent limit to the number of
    files and directories that can be monitored simultaneously.
    If this limit has been reached, the excess \a paths will not
    be monitored, and they will be added to the returned QStringList.

    \sa addPath(), removePaths()
*/
QStringList DFileSystemWatcher::addPaths(const QStringList &paths)
{
    Q_D(DFileSystemWatcher);

    QStringList p = paths;
    QMutableListIterator<QString> it(p);

    while (it.hasNext()) {
        const QString &path = it.next();
        if (path.isEmpty())
            it.remove();
    }

    if (p.isEmpty()) {
        qWarning("DFileSystemWatcher::addPaths: list is empty");
        return QStringList();
    }

    if (d)
        p = d->addPaths(p, &d->files, &d->directories);

    return p;
}

/*!
    Removes the specified \a path from the file system watcher.

    If the watch is successfully removed, true is returned.

    Reasons for watch removal failing are generally system-dependent,
    but may be due to the path having already been deleted, for example.

    \sa removePaths(), addPath()
*/
bool DFileSystemWatcher::removePath(const QString &path)
{
    if (path.isEmpty()) {
        qWarning("DFileSystemWatcher::removePath: path is empty");
        return true;
    }

    QStringList paths = removePaths(QStringList(path));
    return paths.isEmpty();
}

/*!
    Removes the specified \a paths from the file system watcher.

    The return value is a list of paths which were not able to be
    unwatched successfully.

    Reasons for watch removal failing are generally system-dependent,
    but may be due to the path having already been deleted, for example.

    \sa removePath(), addPaths()
*/
QStringList DFileSystemWatcher::removePaths(const QStringList &paths)
{
    Q_D(DFileSystemWatcher);

    QStringList p = paths;
    QMutableListIterator<QString> it(p);

    while (it.hasNext()) {
        const QString &path = it.next();
        if (path.isEmpty())
            it.remove();
    }

    if (p.isEmpty()) {
        qWarning("DFileSystemWatcher::removePaths: list is empty");
        return QStringList();
    }

    if (d)
        p = d->removePaths(p, &d->files, &d->directories);

    return p;
}

/*!
    \fn void DFileSystemWatcher::fileChanged(const QString &path)

    This signal is emitted when the file at the specified \a path is
    modified, renamed or removed from disk.

    \sa directoryChanged()
*/

/*!
    \fn void DFileSystemWatcher::directoryChanged(const QString &path)

    This signal is emitted when the directory at a specified \a path
    is modified (e.g., when a file is added or deleted) or removed
    from disk. Note that if there are several changes during a short
    period of time, some of the changes might not emit this signal.
    However, the last change in the sequence of changes will always
    generate this signal.

    \sa fileChanged()
*/

/*!
    \fn QStringList DFileSystemWatcher::directories() const

    Returns a list of paths to directories that are being watched.

    \sa files()
*/

/*!
    \fn QStringList DFileSystemWatcher::files() const

    Returns a list of paths to files that are being watched.

    \sa directories()
*/

QStringList DFileSystemWatcher::directories() const
{
    Q_D(const DFileSystemWatcher);

    if (!d)
        return QStringList();

    return d->directories;
}

QStringList DFileSystemWatcher::files() const
{
    Q_D(const DFileSystemWatcher);

    if (!d)
        return QStringList();

    return d->files;
}

#include "moc_dfilesystemwatcher.cpp"
