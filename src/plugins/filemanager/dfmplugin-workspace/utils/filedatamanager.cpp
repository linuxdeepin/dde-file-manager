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

    fmDebug() << "Creating new RootInfo for URL:" << url.toString();
    return createRoot(url);
}

bool FileDataManager::fetchFiles(const QUrl &rootUrl, const QString &key, DFMGLOBAL_NAMESPACE::ItemRoles role, Qt::SortOrder order)
{
    fmDebug() << "Starting file fetch for URL:" << rootUrl.toString() << "key:" << key;

    for (auto it = deleteLaterList.begin(); it != deleteLaterList.end();) {
        if ((*it)->canDelete()) {
            (*it)->deleteLater();
            it = deleteLaterList.erase(it);
        } else {
            it++;
        }
    }

    RootInfo *root = rootInfoMap.value(rootUrl);
    if (!root) {
        fmWarning() << "Failed to fetch files - no RootInfo found for URL:" << rootUrl.toString();
        return false;
    }

    auto getCache = root->initThreadOfFileData(key, role, order, isMixFileAndFolder);
    root->startWork(key, getCache);
    fmInfo() << "File fetch started successfully for URL:" << rootUrl.toString() << "with cache:" << getCache;
    return true;
}

void FileDataManager::cleanRoot(const QUrl &rootUrl, const QString &key, const bool refresh, const bool self)
{
    fmDebug() << "Cleaning root for URL:" << rootUrl.toString() << "key:" << key << "refresh:" << refresh << "self:" << self;

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
                if (!root) {
                    fmWarning() << "Failed to retrieve root for cleanup:" << rootInfo.toString();
                    return;
                }
                handleDeletion(root);
            }
        }
    }
}

void FileDataManager::cleanRoot(const QUrl &rootUrl)
{
    fmDebug() << "Performing complete root cleanup for URL:" << rootUrl.toString();

    QString rootPath = rootUrl.path();
    if (!rootPath.endsWith("/"))
        rootPath.append("/");

    auto rootInfoKeys = rootInfoMap.keys();
    for (const auto &rootInfo : rootInfoKeys) {
        QString infoPath = rootInfo.path();
        if (!infoPath.endsWith("/"))
            infoPath.append("/");

        // Compare normalized paths (both with trailing slash) for accurate matching
        if (infoPath.startsWith(rootPath) || infoPath == rootPath) {
            rootInfoMap.value(rootInfo)->disconnect();
            auto root = rootInfoMap.take(rootInfo);
            if (!root) {
                fmWarning() << "Failed to retrieve root for complete cleanup:" << rootInfo.toString();
                return;
            }
            handleDeletion(root);
        }
    }
}

void FileDataManager::stopRootWork(const QUrl &rootUrl, const QString &key)
{
    fmDebug() << "Stopping root work for URL:" << rootUrl.toString() << "key:" << key;

    QString rootPath = rootUrl.path();
    if (!rootPath.endsWith("/"))
        rootPath.append("/");

    auto rootInfoKeys = rootInfoMap.keys();
    for (const auto &rootInfo : rootInfoKeys) {
        if (UniversalUtils::urlEqualsWithQuery(rootInfo, rootUrl) || (rootInfo.path() != rootPath && rootInfo.path().startsWith(rootPath))) {
            if (rootInfoMap.value(rootInfo)->checkKeyOnly(key))
                rootInfoMap.value(rootInfo)->disconnect();
            rootInfoMap.value(rootInfo)->clearTraversalThread(key, false);
        }
    }
}

void FileDataManager::setFileActive(const QUrl &rootUrl, const QUrl &childUrl, bool active)
{
    fmDebug() << "Setting file active state - root:" << rootUrl.toString() << "child:" << childUrl.toString() << "active:" << active;

    RootInfo *root = rootInfoMap.value(rootUrl);
    if (root && root->watcher) {
        root->watcher->setEnabledSubfileWatcher(childUrl, active);
        fmDebug() << "File active state updated successfully";
    } else {
        fmWarning() << "Cannot set file active - root or watcher not found for:" << rootUrl.toString();
    }
}

void FileDataManager::onAppAttributeChanged(Application::ApplicationAttribute aa, const QVariant &value)
{
    if (aa == Application::kFileAndDirMixedSort) {
        bool newValue = value.toBool();
        fmInfo() << "File and directory mixed sort changed from" << isMixFileAndFolder << "to" << newValue;
        isMixFileAndFolder = newValue;
    }
}

void FileDataManager::onHandleFileDeleted(const QUrl url)
{
    fmDebug() << "Handling file deletion request for URL:" << url.toString();
    cleanRoot(url);
}

FileDataManager::FileDataManager(QObject *parent)
    : QObject(parent)
{
    fmDebug() << "FileDataManager initialized";

    isMixFileAndFolder = Application::instance()->appAttribute(Application::kFileAndDirMixedSort).toBool();
    fmDebug() << "Mixed file and folder sorting enabled:" << isMixFileAndFolder;

    connect(Application::instance(), &Application::appAttributeChanged, this, &FileDataManager::onAppAttributeChanged);

    // BUG: 201233 201233
    // TODO: this is workaround
    connect(DevProxyMng, &DeviceProxyManager::mountPointAboutToRemoved, this, [this](QStringView mpt) {
        QUrl url { { QUrl::fromLocalFile(mpt.toString()) } };
        if (!url.isValid()) {
            fmWarning() << "Invalid mount point URL during removal:" << mpt.toString();
            return;
        }
        fmInfo() << "Mount point being removed, cleaning associated data:" << url.toString();
        cleanRoot(url);
        emit InfoCacheController::instance().removeCacheFileInfo({ url });
        WatcherCache::instance().removeCacheWatcherByParent(url);
    });
}

FileDataManager::~FileDataManager()
{
    fmDebug() << "FileDataManager destructor - cleaning up" << rootInfoMap.size() << "roots and" << deleteLaterList.size() << "pending deletions";

    // clean rootInfoMap
    rootInfoMap.clear();
    deleteLaterList.clear();

    fmDebug() << "FileDataManager cleanup completed";
}

RootInfo *FileDataManager::createRoot(const QUrl &url)
{
    bool needCache = checkNeedCache(url);
    fmInfo() << "Creating RootInfo for URL:" << url.toString() << "cache needed:" << needCache;

    // create a new RootInfo
    RootInfo *root = new RootInfo(url, needCache);

    // insert it to rootInfoMap
    rootInfoMap.insert(url, root);
    connect(root, &RootInfo::requestClearRoot, this, &FileDataManager::onHandleFileDeleted,
            Qt::QueuedConnection);

    fmDebug() << "RootInfo created and connected, total roots:" << rootInfoMap.size();
    return root;
}

bool FileDataManager::checkNeedCache(const QUrl &url)
{
    if (cacheDataSchemes.contains(url.scheme())) {
        fmDebug() << "Cache needed - scheme in cache list:" << url.scheme();
        return true;
    }

    // mounted dir should cache files in FileDataManager
    // The purpose is only to judge nonlocal disk files, some schme should not use it to judge, so it is limited to file.
    if (url.scheme() == Global::Scheme::kFile && (!ProtocolUtils::isLocalFile(url))) {
        fmDebug() << "Cache needed - non-local file:" << url.toString();
        return true;
    }

    fmDebug() << "Cache not needed for URL:" << url.toString();
    return false;
}

void FileDataManager::handleDeletion(RootInfo *root)
{
    Q_ASSERT(root);
    if (root->canDelete()) {
        fmDebug() << "RootInfo can be deleted immediately";
        root->deleteLater();
    } else {
        fmDebug() << "RootInfo cannot be deleted immediately, adding to pending list";
        root->reset();
        deleteLaterList.append(root);
    }
}

// NOTE: in tree mode, this func will clean all child node data.
void FileDataManager::cleanUnusedRoots(const QUrl &currentUrl, const QString &key)
{
    if (!currentUrl.isValid()) {
        fmWarning() << "Cannot clean unused roots - invalid current URL";
        return;
    }

    fmInfo() << "Cleaning unused roots, keeping current URL:" << currentUrl.toString() << "key:" << key;

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
