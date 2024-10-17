// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filedatamanager.h"
#include "models/rootinfo.h"
#include "models/fileitemdata.h"
#include "events/workspaceeventcaller.h"

#include <dfm-base/utils/fileutils.h>
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
                if (root)
                    root->deleteLater();
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
            if (root)
                root->deleteLater();
        }
    }
}

void FileDataManager::setFileActive(const QUrl &rootUrl, const QUrl &childUrl, bool active)
{
    RootInfo *root = rootInfoMap.value(rootUrl);
    if (root && root->watcher)
        root->watcher->setEnabledSubfileWatcher(childUrl, active);
}

bool FileDataManager::isMountedDevPath(const QUrl &url)
{
    for (auto iter = allMntedDevs.cbegin(); iter != allMntedDevs.cend(); ++iter) {
        auto urls = allMntedDevs.values(iter.key());
        auto ret = std::find_if(urls.cbegin(), urls.cend(), [url](const QUrl &u) { return DFMBASE_NAMESPACE::UniversalUtils::urlEquals(u, url); });
        if (ret != urls.cend())
            return true;
    }
    return false;
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

void FileDataManager::removeCachedMnts(const QString &id)
{
    if (!allMntedDevs.contains(id))
        return;

    for (const auto &url : allMntedDevs.values(id)) {
        WorkspaceEventCaller::sendCloseTab(url);
        FileDataManager::instance()->cleanRoot(url);
        Q_EMIT InfoCacheController::instance().removeCacheFileInfo({ url });
        WatcherCache::instance().removeCacheWatcherByParent(url);
    }

    allMntedDevs.remove(id);
}

void FileDataManager::cacheMnt(const QString &id, const QString &mnt)
{
    if (!mnt.isEmpty())
        allMntedDevs.insert(id, QUrl::fromLocalFile(mnt));
}

FileDataManager::FileDataManager(QObject *parent)
    : QObject(parent)
{
    isMixFileAndFolder = Application::instance()->appAttribute(Application::kFileAndDirMixedSort).toBool();
    connect(Application::instance(), &Application::appAttributeChanged, this, &FileDataManager::onAppAttributeChanged);
}

FileDataManager::~FileDataManager()
{
    //clean rootInfoMap
    qDeleteAll(rootInfoMap.values());
    rootInfoMap.clear();
}

void FileDataManager::initMntedDevsCache()
{
    QObject::connect(DevProxyMng, &DeviceProxyManager::blockDevMounted, this, &FileDataManager::cacheMnt);
    QObject::connect(DevProxyMng, &DeviceProxyManager::protocolDevMounted, this, &FileDataManager::cacheMnt);
    QObject::connect(DevProxyMng, &DeviceProxyManager::blockDevAdded, this, [this](const QString &id) { cacheMnt(id, ""); });
    QObject::connect(DevProxyMng, &DeviceProxyManager::blockDevUnmounted, this, &FileDataManager::removeCachedMnts);
    QObject::connect(DevProxyMng, &DeviceProxyManager::blockDevRemoved, this, &FileDataManager::removeCachedMnts);
    QObject::connect(DevProxyMng, &DeviceProxyManager::protocolDevUnmounted, this, &FileDataManager::removeCachedMnts);

    for (auto id : DevProxyMng->getAllBlockIds()) {
        auto datas = DevProxyMng->queryBlockInfo(id);
        const QString &&mntPath = datas.value(GlobalServerDefines::DeviceProperty::kMountPoint).toString();
        cacheMnt(id, mntPath);
    }
    for (auto id : DevProxyMng->getAllProtocolIds()) {
        auto datas = DevProxyMng->queryProtocolInfo(id);
        const QString &&mntPath = datas.value(GlobalServerDefines::DeviceProperty::kMountPoint).toString();
        if (!mntPath.isEmpty())
            allMntedDevs.insert(id, QUrl::fromLocalFile(mntPath));
    }
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
    if ((!FileUtils::isLocalDevice(url)))
        return true;

    return false;
}
