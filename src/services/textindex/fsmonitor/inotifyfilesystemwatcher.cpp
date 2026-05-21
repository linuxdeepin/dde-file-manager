// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
// Adapted from DTK DFileSystemWatcher (LGPL-3.0-or-later)

#include "inotifyfilesystemwatcher_p.h"

#include <QFileInfo>
#include <QDir>
#include <QSocketNotifier>
#include <QDebug>
#include <QMultiMap>

#include <sys/inotify.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>

SERVICETEXTINDEX_BEGIN_NAMESPACE

InotifyFileSystemWatcherPrivate::InotifyFileSystemWatcherPrivate(int fd, InotifyFileSystemWatcher *qq)
    : q_ptr(qq), inotifyFd(fd), notifier(fd, QSocketNotifier::Read, qq)
{
    fcntl(inotifyFd, F_SETFD, FD_CLOEXEC);
    QObject::connect(&notifier, &QSocketNotifier::activated, qq, [this]() {
        readFromInotify();
    });
}

InotifyFileSystemWatcherPrivate::~InotifyFileSystemWatcherPrivate()
{
    notifier.setEnabled(false);
    for (int id : std::as_const(pathToID))
        inotify_rm_watch(inotifyFd, id < 0 ? -id : id);

    ::close(inotifyFd);
}

uint32_t InotifyFileSystemWatcherPrivate::toInotifyMask(InotifyFileSystemWatcher::WatchFlags flags, bool isDir)
{
    using WF = InotifyFileSystemWatcher::WatchFlag;
    uint32_t mask = 0;

    // Self-referencing events (IN_DELETE_SELF, IN_MOVE_SELF) are always
    // included so the watcher can react when the watched path itself is
    // removed or renamed, regardless of which event types the consumer
    // has enabled.
    mask |= IN_DELETE_SELF | IN_MOVE_SELF;

    if (flags & WF::FileClose)
        mask |= IN_CLOSE_WRITE;
    if (flags & WF::FileModify)
        mask |= IN_MODIFY;
    if (flags & WF::FileCreate)
        mask |= IN_CREATE;
    if (flags & WF::FileDelete)
        mask |= isDir ? IN_DELETE : 0;
    if (flags & WF::FileMove)
        mask |= isDir ? IN_MOVE : 0;
    if (flags & WF::FileAttribute)
        mask |= IN_ATTRIB;

    return mask;
}

QStringList InotifyFileSystemWatcherPrivate::addPaths(const QStringList &paths, QStringList *files, QStringList *directories)
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

        uint32_t mask = toInotifyMask(watchFlags, isDir);

        int wd = inotify_add_watch(inotifyFd,
                                   QFile::encodeName(path),
                                   mask);
        if (wd < 0) {
            perror("InotifyFileSystemWatcherPrivate::addPaths: inotify_add_watch failed");
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

QStringList InotifyFileSystemWatcherPrivate::removePaths(const QStringList &paths, QStringList *files, QStringList *directories)
{
    QStringList p = paths;
    QMutableListIterator<QString> it(p);
    while (it.hasNext()) {
        QString path = it.next();
        int id = pathToID.take(path);
        for (auto hit = idToPath.find(id); hit != idToPath.end() && hit.key() == id; ++hit) {
            if (hit.value() == path) {
                idToPath.erase(hit);
                break;
            }
        }

        it.remove();

        if (!idToPath.contains(id)) {
            int wd = id < 0 ? -id : id;
            inotify_rm_watch(inotifyFd, wd);
        }

        if (id < 0) {
            directories->removeAll(path);
        } else {
            files->removeAll(path);
        }
    }

    return p;
}

void InotifyFileSystemWatcherPrivate::readFromInotify()
{
    Q_Q(InotifyFileSystemWatcher);

    int buffSize = 0;
    ioctl(inotifyFd, FIONREAD, (char *)&buffSize);
    QVarLengthArray<char, 4096> buffer(buffSize);
    buffSize = read(inotifyFd, buffer.data(), buffSize);
    char *at = buffer.data();
    char *const end = at + buffSize;

    QList<inotify_event *> eventList;
    QMultiHash<int, QString> batch_pathmap;
    QMultiMap<int, QString> cookieToFilePath;
    QMultiMap<int, QString> cookieToFileName;
    QSet<int> hasMoveFromByCookie;

    while (at < end) {
        inotify_event *event = reinterpret_cast<inotify_event *>(at);
        QStringList paths;

        at += sizeof(inotify_event) + event->len;

        int id = event->wd;
        paths = idToPath.values(id);
        if (paths.empty()) {
            // perhaps a directory?
            id = -id;
            paths = idToPath.values(id);
            if (paths.empty())
                continue;
        }

        if (!(event->mask & IN_MOVED_TO) || !hasMoveFromByCookie.contains(event->cookie)) {
            auto it = std::find_if(eventList.begin(), eventList.end(), [event](inotify_event *e) {
                return event->wd == e->wd && event->mask == e->mask
                        && event->cookie == e->cookie
                        && event->len == e->len
                        && !strcmp(event->name, e->name);
            });

            if (it == eventList.end()) {
                eventList.append(event);
            }

            const QList<QString> bps = batch_pathmap.values(id);
            for (auto &path : paths) {
                if (!bps.contains(path)) {
                    batch_pathmap.insert(id, path);
                }
            }
        }

        if (event->mask & IN_MOVED_TO) {
            for (auto &path : paths) {
                cookieToFilePath.insert(event->cookie, path);
            }
            cookieToFileName.insert(event->cookie, QString::fromUtf8(event->name));
        }

        if (event->mask & IN_MOVED_FROM)
            hasMoveFromByCookie << event->cookie;
    }

    QList<inotify_event *>::const_iterator it = eventList.constBegin();
    while (it != eventList.constEnd()) {
        const inotify_event &event = **it;
        ++it;

        int id = event.wd;
        QStringList paths = batch_pathmap.values(id);

        if (paths.empty()) {
            id = -id;
            paths = batch_pathmap.values(id);

            if (paths.empty())
                continue;
        }
        const QString &name = QString::fromUtf8(event.name);

        for (auto &path : paths) {
            if ((event.mask & (IN_DELETE_SELF | IN_MOVE_SELF | IN_UNMOUNT)) != 0) {
                do {
                    if (event.mask & IN_MOVE_SELF) {
                        QMultiMap<int, QString>::const_iterator iterator = cookieToFilePath.constBegin();

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

                    Q_EMIT q->fileDeleted(path, QString());
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
                    filePath = path + QDir::separator() + name;
            }

            if (event.mask & IN_CREATE) {
                if (name.isEmpty()) {
                    if (pathToID.contains(path)) {
                        q->removePath(path);
                        q->addPath(path);
                    }
                } else if (pathToID.contains(filePath)) {
                    q->removePath(filePath);
                    q->addPath(filePath);
                }

                Q_EMIT q->fileCreated(path, name);
            }

            if (event.mask & IN_DELETE) {
                Q_EMIT q->fileDeleted(path, name);
            }

            if (event.mask & IN_MOVED_FROM) {
                const QString toName = cookieToFileName.value(event.cookie);

                if (cookieToFilePath.values(event.cookie).empty()) {
                    Q_EMIT q->fileMoved(path, name, QString(), QString());
                } else {
                    for (QString &toPath : cookieToFilePath.values(event.cookie)) {
                        Q_EMIT q->fileMoved(path, name, toPath, toName);
                    }
                }
            }

            if (event.mask & IN_MOVED_TO) {
                if (!hasMoveFromByCookie.contains(event.cookie))
                    Q_EMIT q->fileMoved(QString(), QString(), path, name);
            }

            if (event.mask & IN_ATTRIB) {
                Q_EMIT q->fileAttributeChanged(path, name);
            }

            // Emitted when a file opened for writing is closed
            // For directories: name is the file within the directory that was closed
            // For files: name is empty
            if (event.mask & IN_CLOSE_WRITE) {
                Q_EMIT q->fileClosed(path, id < 0 ? name : QString());
            }

            if (event.mask & IN_MODIFY) {
                Q_EMIT q->fileModified(path, name);
            }
        }
    }
}

void InotifyFileSystemWatcherPrivate::onFileChanged(const QString &path, bool removed)
{
    if (!files.contains(path)) {
        return;
    }
    if (removed) {
        files.removeAll(path);
    }
}

void InotifyFileSystemWatcherPrivate::onDirectoryChanged(const QString &path, bool removed)
{
    if (!directories.contains(path)) {
        return;
    }
    if (removed) {
        directories.removeAll(path);
    }
}

// ========== InotifyFileSystemWatcher implementation ==========

InotifyFileSystemWatcher::InotifyFileSystemWatcher(QObject *parent)
    : QObject(parent)
{
    int fd = -1;
#ifdef IN_CLOEXEC
    fd = inotify_init1(IN_CLOEXEC | O_NONBLOCK);
#endif
    if (fd == -1) {
        fd = inotify_init1(O_NONBLOCK);
    }

    if (fd != -1) {
        d_ptr.reset(new InotifyFileSystemWatcherPrivate(fd, this));
    } else {
        qCritical() << "inotify_init1 failed, and the InotifyFileSystemWatcher is invalid." << strerror(errno);
    }
}

InotifyFileSystemWatcher::~InotifyFileSystemWatcher()
{
}

void InotifyFileSystemWatcher::setWatchFlags(WatchFlags flags)
{
    Q_D(InotifyFileSystemWatcher);

    if (!d)
        return;

    d->watchFlags = flags;
}

InotifyFileSystemWatcher::WatchFlags InotifyFileSystemWatcher::watchFlags() const
{
    Q_D(const InotifyFileSystemWatcher);

    if (!d)
        return WatchFlags(NoFlags);

    return d->watchFlags;
}

bool InotifyFileSystemWatcher::addPath(const QString &path)
{
    const QStringList &paths = addPaths(QStringList(path));
    return paths.isEmpty();
}

QStringList InotifyFileSystemWatcher::addPaths(const QStringList &paths)
{
    Q_D(InotifyFileSystemWatcher);

    if (!d)
        return paths;

    QStringList p = paths;
    QMutableListIterator<QString> it(p);

    while (it.hasNext()) {
        const QString &path = it.next();
        if (path.isEmpty()) {
            qWarning() << Q_FUNC_INFO << "the path is empty and it is not be watched";
            it.remove();
        }
    }

    if (p.isEmpty()) {
        qWarning() << Q_FUNC_INFO << "all path are filtered and they are not be watched, paths are " << paths;
        return paths;
    }

    p = d->addPaths(p, &d->files, &d->directories);

    return p;
}

bool InotifyFileSystemWatcher::removePath(const QString &path)
{
    const QStringList &paths = removePaths(QStringList(path));
    return paths.isEmpty();
}

QStringList InotifyFileSystemWatcher::removePaths(const QStringList &paths)
{
    Q_D(InotifyFileSystemWatcher);

    if (!d)
        return paths;

    QStringList p = paths;
    QMutableListIterator<QString> it(p);

    while (it.hasNext()) {
        const QString &path = it.next();
        if (path.isEmpty()) {
            qWarning() << Q_FUNC_INFO << "the path is empty and it is not be removed from watched list";
            it.remove();
        }
    }

    if (p.isEmpty()) {
        qWarning() << Q_FUNC_INFO << "all path are filtered and they are not be watched, paths are " << paths;
        return paths;
    }

    p = d->removePaths(p, &d->files, &d->directories);

    return p;
}

QStringList InotifyFileSystemWatcher::directories() const
{
    Q_D(const InotifyFileSystemWatcher);

    if (!d)
        return QStringList();

    return d->directories;
}

QStringList InotifyFileSystemWatcher::files() const
{
    Q_D(const InotifyFileSystemWatcher);

    if (!d)
        return QStringList();

    return d->files;
}

SERVICETEXTINDEX_END_NAMESPACE
