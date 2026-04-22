// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
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

RootInfo *FileDataManager::fetchRoot(const QUrl &url, const QString &key)
{
    const QUrl normalizedUrl = normalizeRootUrl(url);
    addRootUser(normalizedUrl, key);

    if (rootInfoMap.contains(normalizedUrl))
        return rootInfoMap.value(normalizedUrl);

    fmDebug() << "Creating new RootInfo for URL:" << normalizedUrl.toString();
    return createRoot(normalizedUrl);
}

bool FileDataManager::fetchFiles(const QUrl &rootUrl, const QString &key, DFMGLOBAL_NAMESPACE::ItemRoles role, Qt::SortOrder order)
{
    const QUrl normalizedRootUrl = normalizeRootUrl(rootUrl);
    fmDebug() << "Starting file fetch for URL:" << normalizedRootUrl.toString() << "key:" << key;

    for (auto it = deleteLaterList.begin(); it != deleteLaterList.end();) {
        if ((*it)->canDelete()) {
            (*it)->deleteLater();
            it = deleteLaterList.erase(it);
        } else {
            it++;
        }
    }

    RootInfo *root = rootInfoMap.value(normalizedRootUrl);
    if (!root) {
        fmWarning() << "No RootInfo found, attempting to recreate for URL:" << normalizedRootUrl.toString();
        root = fetchRoot(normalizedRootUrl, key);
        if (!root) {
            fmWarning() << "Failed to recreate RootInfo for URL:" << normalizedRootUrl.toString();
            return false;
        }
    }

    root->initThreadOfFileData(key, role, order, isMixFileAndFolder);
    root->startWork(key);
    fmInfo() << "File fetch started successfully for URL:" << normalizedRootUrl.toString();
    return true;
}

void FileDataManager::cleanRoot(const QUrl &rootUrl, const QString &key, const bool refresh, const bool self)
{
    const QUrl normalizedRootUrl = normalizeRootUrl(rootUrl);
    fmDebug() << "Cleaning root for URL:" << normalizedRootUrl.toString() << "key:" << key << "refresh:" << refresh << "self:" << self;

    QString rootPath = normalizedRootUrl.path();
    if (!rootPath.endsWith("/"))
        rootPath.append("/");

    auto rootInfoKeys = rootInfoMap.keys();
    for (const auto &rootInfo : rootInfoKeys) {
        if (!self && normalizedRootUrl == rootInfo)
            continue;

        if (rootInfo.path().startsWith(rootPath) || rootInfo.path() == normalizedRootUrl.path()) {
            removeRootUser(rootInfo, key);
            auto count = rootInfoMap.value(rootInfo)->clearTraversalThread(key, refresh);
            if (count > 0)
                continue;
            if (!hasRootUsers(rootInfo)) {
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
    const QUrl normalizedRootUrl = normalizeRootUrl(rootUrl);
    fmDebug() << "Performing complete root cleanup for URL:" << normalizedRootUrl.toString();

    QString rootPath = normalizedRootUrl.path();
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
            rootUsers.remove(rootInfo);
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
    const QUrl normalizedRootUrl = normalizeRootUrl(rootUrl);
    fmDebug() << "Stopping root work for URL:" << normalizedRootUrl.toString() << "key:" << key;

    QString rootPath = normalizedRootUrl.path();
    if (!rootPath.endsWith("/"))
        rootPath.append("/");

    auto rootInfoKeys = rootInfoMap.keys();
    for (const auto &rootInfo : rootInfoKeys) {
        if (UniversalUtils::urlEqualsWithQuery(rootInfo, normalizedRootUrl) || (rootInfo.path() != rootPath && rootInfo.path().startsWith(rootPath))) {
            if (rootInfoMap.value(rootInfo)->checkKeyOnly(key))
                rootInfoMap.value(rootInfo)->disconnect();
            rootInfoMap.value(rootInfo)->clearTraversalThread(key, false);
        }
    }
}

void FileDataManager::setFileActive(const QUrl &rootUrl, const QUrl &childUrl, bool active)
{
    const QUrl normalizedRootUrl = normalizeRootUrl(rootUrl);
    fmDebug() << "Setting file active state - root:" << normalizedRootUrl.toString() << "child:" << childUrl.toString() << "active:" << active;

    RootInfo *root = rootInfoMap.value(normalizedRootUrl);
    if (root && root->watcher) {
        root->watcher->setEnabledSubfileWatcher(childUrl, active);
        fmDebug() << "File active state updated successfully";
    } else {
        fmWarning() << "Cannot set file active - root or watcher not found for:" << normalizedRootUrl.toString();
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
    const QUrl normalizedUrl = normalizeRootUrl(url);
    fmInfo() << "Creating RootInfo for URL:" << normalizedUrl.toString();

    // create a new RootInfo
    RootInfo *root = new RootInfo(normalizedUrl);

    // insert it to rootInfoMap
    rootInfoMap.insert(normalizedUrl, root);
    connect(root, &RootInfo::requestClearRoot, this, &FileDataManager::onHandleFileDeleted,
            Qt::QueuedConnection);

    fmDebug() << "RootInfo created and connected, total roots:" << rootInfoMap.size();
    return root;
}

QUrl FileDataManager::normalizeRootUrl(const QUrl &url) const
{
    if (!url.isValid())
        return url;

    QUrl normalizedUrl(url);
    QString path = normalizedUrl.path();
    if (path.isEmpty()) {
        path = "/";
    } else if (path.length() > 1 && path.endsWith("/")) {
        path.chop(1);
    }
    normalizedUrl.setPath(path);
    return normalizedUrl;
}

void FileDataManager::addRootUser(const QUrl &url, const QString &key)
{
    if (key.isEmpty())
        return;

    rootUsers[url].insert(key);
}

void FileDataManager::removeRootUser(const QUrl &url, const QString &key)
{
    if (key.isEmpty())
        return;

    auto it = rootUsers.find(url);
    if (it == rootUsers.end())
        return;

    it->remove(key);
    if (it->isEmpty())
        rootUsers.erase(it);
}

bool FileDataManager::hasRootUsers(const QUrl &url) const
{
    auto it = rootUsers.constFind(url);
    return it != rootUsers.constEnd() && !it->isEmpty();
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
    const QUrl normalizedCurrentUrl = normalizeRootUrl(currentUrl);
    if (!normalizedCurrentUrl.isValid()) {
        fmWarning() << "Cannot clean unused roots - invalid current URL";
        return;
    }

    fmInfo() << "Cleaning unused roots, keeping current URL:" << normalizedCurrentUrl.toString() << "key:" << key;

    // 确保路径以/结尾，用于目录比较
    QString currentPath = normalizedCurrentUrl.path();
    if (!currentPath.endsWith("/"))
        currentPath.append("/");

    QList<QUrl> rootsToClean;

    // 找出所有需要清理的RootInfo
    for (const auto &rootUrl : rootInfoMap.keys()) {
        // 跳过当前URL
        if (UniversalUtils::urlEqualsWithQuery(rootUrl, normalizedCurrentUrl))
            continue;

        rootsToClean.append(rootUrl);
    }

    // 清理标记的RootInfo
    for (const auto &rootUrl : rootsToClean) {
        removeRootUser(rootUrl, key);

        // 先停止线程工作
        int rootThreadCount = rootInfoMap.value(rootUrl)->clearTraversalThread(key, false);

        // 剩余迭代线程大于0或仍有视图使用该rootinfo，不能直接移除
        if (rootThreadCount > 0 || hasRootUsers(rootUrl))
            continue;

        // 从Map中移除并处理删除
        auto root = rootInfoMap.take(rootUrl);
        if (root) {
            handleDeletion(root);
        }
    }
}
