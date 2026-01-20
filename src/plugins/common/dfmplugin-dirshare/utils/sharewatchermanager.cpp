// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sharewatchermanager.h"

#include <dfm-base/file/local/localfilewatcher.h>
#include <dfm-base/utils/finallyutil.h>

#include <dfm-io/dfmio_utils.h>

using namespace dfmplugin_dirshare;
DFMBASE_USE_NAMESPACE

ShareWatcherManager::ShareWatcherManager(QObject *parent)
    : QObject(parent)
{
}

ShareWatcherManager::~ShareWatcherManager()
{
    for (auto &watcher : watchers) {
        if (watcher) {
            watcher->stopWatcher();
        }
    }
    for (auto &list : parentWatchers) {
        for (auto &watcher : list) {
            if (watcher) {
                watcher->stopWatcher();
            }
        }
    }
}

QSharedPointer<DFMBASE_NAMESPACE::LocalFileWatcher> ShareWatcherManager::add(const QString &path)
{
    auto watcher = watchers.value(path);
    if (watcher)
        return watcher;

    watcher = QSharedPointer<DFMBASE_NAMESPACE::LocalFileWatcher>(new LocalFileWatcher(QUrl::fromLocalFile(path)));

    if (!watcher) {
        qWarning() << "Failed to create watcher for:" << path;
        return nullptr;
    }

    connect(watcher.data(), &LocalFileWatcher::fileAttributeChanged, this, [this](const QUrl &url) {
        if (shouldEmitSignal(url))
            Q_EMIT this->fileAttributeChanged(url.toLocalFile());
    });
    connect(watcher.data(), &LocalFileWatcher::fileDeleted, this, [this](const QUrl &url) {
        if (shouldEmitSignal(url))
            Q_EMIT this->fileDeleted(url.toLocalFile());
    });
    connect(watcher.data(), &LocalFileWatcher::subfileCreated, this, [this](const QUrl &url) {
        if (shouldEmitSignal(url))
            Q_EMIT this->subfileCreated(url.toLocalFile());
    });
    connect(watcher.data(), &LocalFileWatcher::fileRename, this, [this](const QUrl &oldUrl, const QUrl &newUrl) {
        if (shouldEmitSignal(oldUrl))
            Q_EMIT this->fileMoved(oldUrl.toLocalFile(), newUrl.toLocalFile());
    });

    if (!watcher->startWatcher()) {
        qWarning() << "Failed to start watcher for:" << path;
        return nullptr;
    }

    watchers.insert(path, watcher);

    // 处理父目录监视器问题
    if (path != ShareConfig::kShareConfigPath)
        addParentPathsWatcher(path);
    return watcher;
}

void ShareWatcherManager::remove(const QString &path)
{
    auto watcher = watchers.take(path);
    // 处理父目录监视器问题
    removeParentPathsWatcher(path);
}

void ShareWatcherManager::addParentPathsWatcher(const QString &path)
{
    auto rootPath = dfmio::DFMUtils::mountPathFromUrl(QUrl::fromLocalFile(path));
    if (rootPath.isEmpty()) {
        qWarning() << "Failed to get mount path for:" << path;
        return;
    }
    if (path == rootPath || path == (rootPath + QDir::separator()))
        return;

    // 使用 QDir 处理路径，提高可读性和效率
    QDir rootDir(rootPath);
    QDir targetDir(path);

    QString relativePath = rootDir.relativeFilePath(path);
    QStringList parts = relativePath.split(QDir::separator(), Qt::SkipEmptyParts);

    QString currentPath = rootPath;
    for (const auto &part : parts) {
        currentPath = QDir::cleanPath(currentPath + QDir::separator() + part);
        addParentWatcher(path, currentPath);
    }
}

void ShareWatcherManager::removeParentPathsWatcher(const QString &path)
{
    parentWatchers.remove(path);
}

void ShareWatcherManager::addParentWatcher(const QString &path, const QString &parentPath)
{
    auto add = [=](const QSharedPointer<DFMBASE_NAMESPACE::LocalFileWatcher> &watcher){
        if (watcher.isNull())
            return;
        auto ps = parentWatchers.value(path);
        if (ps.contains(watcher))
            return;
        ps.append(watcher);
        parentWatchers.insert(path, ps);
    };

    // 找当前监视的父目录
    for (const auto &ps : parentWatchers.values()) {
        for (const auto &p : ps) {
            if (p && p->url().toLocalFile() == parentPath) {
                add(p);
                return;
            }
        }
    }

    // 找当前监视的共享目录
    auto watcher = watchers.value(parentPath);

    if (watcher.isNull())
        watcher = QSharedPointer<DFMBASE_NAMESPACE::LocalFileWatcher>(new LocalFileWatcher(QUrl::fromLocalFile(parentPath)));

    connect(watcher.data(), &LocalFileWatcher::fileDeleted, this, [this](const QUrl &url) {
        // 删除的目录是任意一个当前共享目录的父目录，发送当前共享目录被删除信号
        const QString localPath = url.toLocalFile();
        for (const auto &path : watchers.keys()) {
            if (path == localPath || path.startsWith(localPath + QDir::separator()))
                Q_EMIT this->fileDeleted(path);
        }
    });

    connect(watcher.data(), &LocalFileWatcher::fileRename, this, [this](const QUrl &oldUrl, const QUrl &newUrl) {
        Q_UNUSED(newUrl);
        // 删除的目录是任意一个当前共享目录的父目录，发送当前共享目录被删除信号
        for (const auto &path : watchers.keys()) {
            const QString localPath = oldUrl.toLocalFile();
            if (path == localPath || path.startsWith(localPath + QDir::separator()))
                Q_EMIT this->fileDeleted(path);
        }
    });

    add(watcher);
    if (!watcher->startWatcher()) {
        qWarning() << "Failed to start watcher for:" << path;
        return;
    }
}

bool ShareWatcherManager::shouldEmitSignal(const QUrl &url) const
{
    return url.toLocalFile().startsWith(ShareConfig::kShareConfigPath) || watchers.contains(url.toLocalFile());
}
