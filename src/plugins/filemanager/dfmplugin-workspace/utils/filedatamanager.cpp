// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filedatamanager.h"
#include "models/rootinfo.h"
#include "models/fileitemdata.h"
#include "events/workspaceeventcaller.h"

#include <dfm-base/utils/protocolutils.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/utils/watchercache.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/device/deviceproxymanager.h>

#include <QApplication>

using namespace dfmbase;
using namespace dfmplugin_workspace;

FileDataManager *FileDataManager::instance()
{
    static FileDataManager ins;
    return &ins;
}

RootInfo *FileDataManager::fetchRoot(const QUrl &url)
{
    if (rootInfoMap.contains(url))
        return rootInfoMap.value(url);

    return createRoot(url);
}

bool FileDataManager::fetchFiles(const QUrl &rootUrl, const QString &key, DFMGLOBAL_NAMESPACE::ItemRoles role, Qt::SortOrder order)
{
    for (auto it = deleteLaterList.begin(); it != deleteLaterList.end();) {
        if ((*it)->canDelete()) {
            (*it)->deleteLater();
            it = deleteLaterList.erase(it);
        } else {
            it++;
        }
    }

    RootInfo *root = rootInfoMap.value(rootUrl);
    if (!root)
        return false;
    auto getCache = root->initThreadOfFileData(key, role, order, isMixFileAndFolder);
    root->startWork(key, getCache);
    return true;
}

void FileDataManager::cleanRoot(const QUrl &rootUrl, const QString &key, const bool refresh, const bool self)
{
    QString rootPath = rootUrl.path();
    if (!rootPath.endsWith("/"))
        rootPath.append("/");

    auto rootInfoKeys = rootInfoMap.keys();
    for (const auto &rootInfo : rootInfoKeys) {
        if (!self && rootUrl == rootInfo)
            continue;

        if (rootInfo.path().startsWith(rootPath) || rootInfo.path() == rootUrl.path()) {
            auto count = rootInfoMap.value(rootInfo)->clearTraversalThread(key, refresh);
            if (count > 0)
                continue;
            if (!checkNeedCache(rootInfo) || refresh) {
                auto root = rootInfoMap.take(rootInfo);
                if (!root)
                    return;
                handleDeletion(root);
            }
        }
    }
}

void FileDataManager::cleanRoot(const QUrl &rootUrl)
{
    QString rootPath = rootUrl.path();
    if (!rootPath.endsWith("/"))
        rootPath.append("/");

    auto rootInfoKeys = rootInfoMap.keys();
    for (const auto &rootInfo : rootInfoKeys) {
        if (rootInfo.path().startsWith(rootPath) || rootInfo.path() == rootUrl.path()) {
            rootInfoMap.value(rootInfo)->disconnect();
            auto root = rootInfoMap.take(rootInfo);
            if (!root)
                return;
            handleDeletion(root);
        }
    }
}

void FileDataManager::stopRootWork(const QUrl &rootUrl, const QString &key)
{
    QString rootPath = rootUrl.path();
    if (!rootPath.endsWith("/"))
        rootPath.append("/");

    auto rootInfoKeys = rootInfoMap.keys();
    for (const auto &rootInfo : rootInfoKeys) {
        if (UniversalUtils::urlEqualsWithQuery(rootInfo, rootUrl) || (rootInfo.path() != rootPath && rootInfo.path().startsWith(rootPath))) {
            rootInfoMap.value(rootInfo)->disconnect();
            rootInfoMap.value(rootInfo)->clearTraversalThread(key, false);
        }
    }
}

void FileDataManager::setFileActive(const QUrl &rootUrl, const QUrl &childUrl, bool active)
{
    RootInfo *root = rootInfoMap.value(rootUrl);
    if (root && root->watcher)
        root->watcher->setEnabledSubfileWatcher(childUrl, active);
}

void FileDataManager::onAppAttributeChanged(Application::ApplicationAttribute aa, const QVariant &value)
{
    if (aa == Application::kFileAndDirMixedSort)
        isMixFileAndFolder = value.toBool();
}

void FileDataManager::onHandleFileDeleted(const QUrl url)
{
    cleanRoot(url);
}

FileDataManager::FileDataManager(QObject *parent)
    : QObject(parent)
{
    isMixFileAndFolder = Application::instance()->appAttribute(Application::kFileAndDirMixedSort).toBool();
    connect(Application::instance(), &Application::appAttributeChanged, this, &FileDataManager::onAppAttributeChanged);

    // BUG: 201233 201233
    // TODO: this is workaround
    connect(DevProxyMng, &DeviceProxyManager::mountPointAboutToRemoved, this, [this](QStringView mpt) {
        QUrl url { { QUrl::fromLocalFile(mpt.toString()) } };
        if (!url.isValid())
            return;
        cleanRoot(url);
        emit InfoCacheController::instance().removeCacheFileInfo({ url });
        WatcherCache::instance().removeCacheWatcherByParent(url);
    });
}

FileDataManager::~FileDataManager()
{
    // clean rootInfoMap
    rootInfoMap.clear();
    deleteLaterList.clear();
}

RootInfo *FileDataManager::createRoot(const QUrl &url)
{
    // create a new RootInfo
    RootInfo *root = new RootInfo(url, checkNeedCache(url));

    // insert it to rootInfoMap
    rootInfoMap.insert(url, root);
    connect(root, &RootInfo::requestClearRoot, this, &FileDataManager::onHandleFileDeleted,
            Qt::QueuedConnection);

    return root;
}

bool FileDataManager::checkNeedCache(const QUrl &url)
{
    if (cacheDataSchemes.contains(url.scheme()))
        return true;

    // mounted dir should cache files in FileDataManager
    // The purpose is only to judge nonlocal disk files, some schme should not use it to judge, so it is limited to file.
    if (url.scheme() == Global::Scheme::kFile && (!ProtocolUtils::isLocalFile(url)))
        return true;

    return false;
}

void FileDataManager::handleDeletion(RootInfo *root)
{
    Q_ASSERT(root);
    if (root->canDelete()) {
        root->deleteLater();
    } else {
        root->reset();
        deleteLaterList.append(root);
    }
}

// NOTE: in tree mode, this func will clean all child node data.
void FileDataManager::cleanUnusedRoots(const QUrl &currentUrl, const QString &key)
{
    if (!currentUrl.isValid())
        return;
        
    // 确保路径以/结尾，用于目录比较
    QString currentPath = currentUrl.path();
    if (!currentPath.endsWith("/"))
        currentPath.append("/");
        
    QList<QUrl> rootsToClean;
    
    // 找出所有需要清理的RootInfo
    for (const auto &rootUrl : rootInfoMap.keys()) {
        // 跳过当前URL
        if (UniversalUtils::urlEqualsWithQuery(rootUrl, currentUrl))
            continue;

        rootsToClean.append(rootUrl);
    }
    
    // 清理标记的RootInfo
    for (const auto &rootUrl : rootsToClean) {
        // 先停止线程工作
        int rootThreadCount = rootInfoMap.value(rootUrl)->clearTraversalThread(key, false);

        // 剩余迭代线程大于0,说明该rootinfo还在被其他view-model使用，不能直接移除
        if (rootThreadCount > 0)
            continue;

        // 从Map中移除并处理删除
        auto root = rootInfoMap.take(rootUrl);
        if (root) {
            handleDeletion(root);
        }
    }
}
