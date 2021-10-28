/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "dfilewatcher.h"
#include "private/dabstractfilewatcher_p.h"

#include "dfileservices.h"
#include "dfilesystemwatcher.h"

#include "private/dfilesystemwatcher_p.h"
#include "../vault/vaultglobaldefine.h"
#include "controllers/vaultcontroller.h"
#include "app/filesignalmanager.h"
#include "utils/singleton.h"
#include "app/define.h"
#include <QDir>
#include <QDebug>

static QString joinFilePath(const QString &path, const QString &name)
{
    if (path.endsWith(QDir::separator()))
        return path + name;

    return path + QDir::separator() + name;
}

class DFileWatcherPrivate : DAbstractFileWatcherPrivate
{
public:
    explicit DFileWatcherPrivate(DFileWatcher *qq)
        : DAbstractFileWatcherPrivate(qq) {}

    bool start() override;
    bool stop() override;
    bool handleGhostSignal(const DUrl &targetUrl, DAbstractFileWatcher::SignalType1 signal, const DUrl &arg1) override;
    bool handleGhostSignal(const DUrl &targetUrl, DAbstractFileWatcher::SignalType2 signal, const DUrl &arg1, const DUrl &arg2) override;

    void _q_handleFileDeleted(const QString &path, const QString &parentPath);
    void _q_handleFileAttributeChanged(const QString &path, const QString &parentPath);
    bool _q_handleFileMoved(const QString &from, const QString &fromParent, const QString &to, const QString &toParent);
    void _q_handleFileCreated(const QString &path, const QString &parentPath);
    void _q_handleFileModified(const QString &path, const QString &parentPath);
    void _q_handleFileClose(const QString &path, const QString &parentPath);

    static QString formatPath(const QString &path);

    QString path;
    QStringList watchFileList;

    static QMap<QString, int> filePathToWatcherCount;

    Q_DECLARE_PUBLIC(DFileWatcher)
};

QMap<QString, int> DFileWatcherPrivate::filePathToWatcherCount;
Q_GLOBAL_STATIC(DFileSystemWatcher, watcher_file_private)

bool isPathWatched(const QString &path);

QStringList parentPathList(const QString &path)
{
    QStringList list;
    QDir dir(path);

    list << path;

    QString strTmpPath = path;
    // fix bug#27870 往已刻录的文件夹中的文件夹...中添加文件夹，界面不刷新
    // 往已刻录的文件夹中放置文件时，由于父路径可能不存在，导致不能创建对应的 watcher ，因此不能监听到文件夹变化，导致界面不刷新
    if (!dir.exists() && path.contains("/.cache/deepin/discburn/")) // 目前仅针对光驱刻录的暂存区进行处理
        dir.mkdir(path);

    while (dir.cdUp()) {
        //! fixed the bug that directory not refresh when same pathes in the list.
        if (isPathWatched(dir.absolutePath())) {
            continue;
        }
        list << dir.absolutePath();
    }

    return list;
}

bool isPathWatched(const QString &path)
{
    QFileInfo fi(path);
    return (fi.isDir() && watcher_file_private->directories().contains(path)) ||
           (fi.isFile() && watcher_file_private->files().contains(path));
}

bool DFileWatcherPrivate::start()
{
    Q_Q(DFileWatcher);

    started = true;

    foreach (const QString &path, parentPathList(this->path)) {
        if (watchFileList.contains(path))
            continue;

        if (filePathToWatcherCount.value(path, -1) <= 0 || !isPathWatched(path)) {
            if (QFile::exists(path)) {
                bool shouldAddToPath = true;
                // vault directory should not be watched before filesystem mounted.
                if (VaultController::isVaultFile(path) &&
                        (VaultController::ins()->state() != VaultController::Unlocked)) {
                    shouldAddToPath = false;
                }
                if (shouldAddToPath && !watcher_file_private->addPath(path)) {
                    qWarning() << Q_FUNC_INFO << "start watch failed, file path =" << path;
                    q->stopWatcher();
                    started = false;
                    return false;
                }
            }
        }

        watchFileList << path;
        filePathToWatcherCount[path] = filePathToWatcherCount.value(path, 0) + 1;
    }

    q->connect(watcher_file_private, &DFileSystemWatcher::fileDeleted,
               q, &DFileWatcher::onFileDeleted);
    q->connect(watcher_file_private, &DFileSystemWatcher::fileAttributeChanged,
               q, &DFileWatcher::onFileAttributeChanged);
    q->connect(watcher_file_private, &DFileSystemWatcher::fileMoved,
               q, &DFileWatcher::onFileMoved);
    q->connect(watcher_file_private, &DFileSystemWatcher::fileCreated,
               q, &DFileWatcher::onFileCreated);
    q->connect(watcher_file_private, &DFileSystemWatcher::fileModified,
               q, &DFileWatcher::onFileModified);
    q->connect(watcher_file_private, &DFileSystemWatcher::fileClosed,
               q, &DFileWatcher::onFileClosed);
    q->connect(watcher_file_private, &DFileSystemWatcher::fileSystemUMount,
               q, &DFileWatcher::onFileSystemUMount);

    q->connect(fileSignalManager, &FileSignalManager::fileMoved,
               q, &DFileWatcher::onFileMoved);
    return true;
}

bool DFileWatcherPrivate::stop()
{
    Q_Q(DFileWatcher);

    if (watcher_file_private.isDestroyed())
        return true;

//    q->disconnect(watcher_file_private, 0, q, 0);//避免0值警告
    q->disconnect(watcher_file_private, nullptr, q, nullptr);

    bool ok = true;

    for (auto it = watchFileList.begin(); it != watchFileList.end();) {
        int count = filePathToWatcherCount.value(*it, 0);

        --count;
        if (count > 0 && !isPathWatched(*it)) { // already removed from DFileSystemWatcher
            filePathToWatcherCount.remove(*it);
            it = watchFileList.erase(it);
            continue;
        }

        if (count <= 0) {
            filePathToWatcherCount.remove(*it);
            ok = ok && watcher_file_private->removePath(*it);
            it = watchFileList.erase(it);
            continue;
        } else {
            filePathToWatcherCount[*it] = count;
        }

        ++it;
    }

    return ok;
}

bool DFileWatcherPrivate::handleGhostSignal(const DUrl &targetUrl, DAbstractFileWatcher::SignalType1 signal, const DUrl &arg1)
{
    if (!targetUrl.isLocalFile() && !arg1.isMTPFile()) // 华为版本，MTP删除文件自行刷新
        return false;

//    Q_Q(DFileWatcher); //没有被使用

    if (signal == &DAbstractFileWatcher::fileDeleted) {
        if (arg1.isMTPFile()) {
            q_ptr->fileDeleted(arg1);
            return true;
        } else {
            for (const QString &path : watchFileList) {
                const DUrl &_url = DUrl::fromLocalFile(path);

                if (_url == arg1) {
                    q_ptr->fileDeleted(this->url);

                    return true;
                }
            }
        }
    } else {
        return DAbstractFileWatcherPrivate::handleGhostSignal(targetUrl, signal, arg1);
    }

    return false;
}

bool DFileWatcherPrivate::handleGhostSignal(const DUrl &targetUrl, DAbstractFileWatcher::SignalType2 signal, const DUrl &arg1, const DUrl &arg2)
{
    if (!targetUrl.isLocalFile())
        return false;

    if (signal != &DAbstractFileWatcher::fileMoved) {
        return DAbstractFileWatcherPrivate::handleGhostSignal(targetUrl, signal, arg1, arg2);
    }

    return _q_handleFileMoved(arg1.toLocalFile(), arg1.parentUrl().toLocalFile(), arg2.toLocalFile(), arg2.parentUrl().toLocalFile());
}

void DFileWatcherPrivate::_q_handleFileDeleted(const QString &path, const QString &parentPath)
{
    Q_Q(DFileWatcher);

    //如果被删除目录是当前目录的父级及以上目录
    //则需要发出当前目录也被删除的事件
    // fix 99280
    // 原判断会造成误删 例如删除 /media/uos/vfat时，会将 /media/uos/vfat1一起删除了
    const QUrl &currentUrl = QUrl(path);
    const QUrl &thisPathUrl = QUrl(this->path);
    if (currentUrl.isParentOf(thisPathUrl)) {
        emit q->fileDeleted(DUrl::fromLocalFile(this->path));
        return;
    }

    //如果被删除的目录是当前目录或当前目录的子目录
    //则需要发出被删除目录已被删除的事件
    if (path != this->path && parentPath != this->path)
        return;

    emit q->fileDeleted(DUrl::fromLocalFile(path));
}

void DFileWatcherPrivate::_q_handleFileAttributeChanged(const QString &path, const QString &parentPath)
{
    if (path != this->path && parentPath != this->path)
        return;

    Q_Q(DFileWatcher);

    emit q->fileAttributeChanged(DUrl::fromLocalFile(path));
}

bool DFileWatcherPrivate::_q_handleFileMoved(const QString &from, const QString &fromParent, const QString &to, const QString &toParent)
{
    Q_Q(DFileWatcher);

    if ((fromParent == this->path && toParent == this->path) || from == this->path) {
        emit q->fileMoved(DUrl::fromLocalFile(from), DUrl::fromLocalFile(to));
    } else if (fromParent == this->path) {
        emit q->fileDeleted(DUrl::fromLocalFile(from));
    } else if (watchFileList.contains(from)) {
        emit q->fileDeleted(url);
    } else if (toParent == this->path) {
        emit q->subfileCreated(DUrl::fromLocalFile(to));
    } else {
        return false;
    }

    return true;
}

void DFileWatcherPrivate::_q_handleFileCreated(const QString &path, const QString &parentPath)
{
    if (watchFileList.contains(path)) {
        bool result = watcher_file_private->addPath(path);
        if (!result) {
            qWarning() << Q_FUNC_INFO << "add to watcher failed, file path =" << path;
        }
    }

    if (path != this->path && parentPath != this->path)
        return;

    Q_Q(DFileWatcher);

    emit q->subfileCreated(DUrl::fromLocalFile(path));
}

void DFileWatcherPrivate::_q_handleFileModified(const QString &path, const QString &parentPath)
{
    //if (path != this->path && parentPath != this->path)
    // bug 25533: some path add some external path when modified, so use the contain function
    if (!path.contains(this->path) && parentPath != this->path)
        return;

    Q_Q(DFileWatcher);

    emit q->fileModified(DUrl::fromLocalFile(path));
}

void DFileWatcherPrivate::_q_handleFileClose(const QString &path, const QString &parentPath)
{
    if (path != this->path && parentPath != this->path)
        return;

    Q_Q(DFileWatcher);

    emit q->fileClosed(DUrl::fromLocalFile(path));
}

QString DFileWatcherPrivate::formatPath(const QString &path)
{
    QString p = QFileInfo(path).absoluteFilePath();

    if (p.endsWith(QDir::separator()))
        p.chop(1);

    return p.isEmpty() ? path : p;
}

DFileWatcher::DFileWatcher(const QString &filePath, QObject *parent)
    : DAbstractFileWatcher(*new DFileWatcherPrivate(this), DUrl::fromLocalFile(filePath), parent)
{
    d_func()->path = DFileWatcherPrivate::formatPath(filePath);
}

void DFileWatcher::onFileDeleted(const QString &path, const QString &name)
{
    // 为防止文管卡死，保险箱里文件删除不执行后续流程
    // 只有删除保险箱文件时，才不执行后续流程
    if (VaultController::isBigFileDeleting()) {
        return;
    }

    if (name.isEmpty())
        d_func()->_q_handleFileDeleted(path, QString());
    else
        d_func()->_q_handleFileDeleted(joinFilePath(path, name), path);
}

void DFileWatcher::onFileAttributeChanged(const QString &path, const QString &name)
{
    if (name.isEmpty())
        d_func()->_q_handleFileAttributeChanged(path, QString());
    else
        d_func()->_q_handleFileAttributeChanged(joinFilePath(path, name), path);
}

void DFileWatcher::onFileMoved(const QString &from, const QString &fname, const QString &to, const QString &tname)
{
    QString fromPath, fpPath;
    QString toPath, tpPath;

    if (fname.isEmpty()) {
        fromPath = from;
    } else {
        fromPath = joinFilePath(from, fname);
        fpPath = from;
    }

    if (tname.isEmpty()) {
        toPath = to;
    } else {
        toPath = joinFilePath(to, tname);
        tpPath = to;
    }

    d_func()->_q_handleFileMoved(fromPath, fpPath, toPath, tpPath);
}

void DFileWatcher::onFileCreated(const QString &path, const QString &name)
{
    d_func()->_q_handleFileCreated(joinFilePath(path, name), path);
}

void DFileWatcher::onFileModified(const QString &path, const QString &name)
{
    if (name.isEmpty())
        d_func()->_q_handleFileModified(path, QString());
    else
        d_func()->_q_handleFileModified(joinFilePath(path, name), path);
}

void DFileWatcher::onFileClosed(const QString &path, const QString &name)
{
    if (name.isEmpty())
        d_func()->_q_handleFileClose(path, QString());
    else
        d_func()->_q_handleFileClose(joinFilePath(path, name), path);
}

void DFileWatcher::onFileSystemUMount(const QString &path, const QString &name)
{
    Q_UNUSED(name)

    d_func()->filePathToWatcherCount.remove(path);
    watcher_file_private->removePath(path);
    d_func()->watchFileList.removeOne(path);
}

QStringList DFileWatcher::getMonitorFiles()
{
    QStringList list;

    list << watcher_file_private->directories();
    list << watcher_file_private->files();

    list << "---------------------------";

    QMap<QString, int>::const_iterator i = DFileWatcherPrivate::filePathToWatcherCount.constBegin();

    while (i != DFileWatcherPrivate::filePathToWatcherCount.constEnd()) {
        list << QString("%1, %2").arg(i.key()).arg(i.value());
        ++i;
    }

    return list;
}

#include "moc_dfilewatcher.cpp"
