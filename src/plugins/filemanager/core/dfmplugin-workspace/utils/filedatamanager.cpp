// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filedatamanager.h"
#include "models/rootinfo.h"
#include "models/fileitemdata.h"

#include "dfm-base/utils/fileutils.h"

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
        return rootInfoMap[url];

    return createRoot(url);
}

bool FileDataManager::fetchFiles(const QUrl &rootUrl, const QString &key, DFMGLOBAL_NAMESPACE::ItemRoles role, Qt::SortOrder order)
{
    RootInfo *root = rootInfoMap[rootUrl];
    if (!root)
        return false;
    auto getCache = root->initThreadOfFileData(key, role, order, isMixFileAndFolder);
    root->startWork(key, getCache);
    return true;
}

void FileDataManager::cleanRoot(const QUrl &rootUrl, const QString &key, const bool refresh)
{
    if (!rootInfoMap.contains(rootUrl))
        return;
    if (!rootInfoMap.value(rootUrl))
        return;
    auto count = rootInfoMap.value(rootUrl)->clearTraversalThread(key);
    if (count > 0 || refresh)
        return;

    if (!checkNeedCache(rootUrl)) {
        rootInfoMap.value(rootUrl)->setParent(nullptr);
        rootInfoMap.take(rootUrl)->deleteLater();
    }
}

void FileDataManager::setFileActive(const QUrl &rootUrl, const QUrl &childUrl, bool active)
{
    RootInfo *root = rootInfoMap[rootUrl];
    if (root && root->watcher)
        root->watcher->setEnabledSubfileWatcher(childUrl, active);
}

void FileDataManager::onAppAttributeChanged(Application::ApplicationAttribute aa, const QVariant &value)
{
    if (aa == Application::kFileAndDirMixedSort)
        isMixFileAndFolder = value.toBool();
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

RootInfo *FileDataManager::createRoot(const QUrl &url)
{
    // create a new RootInfo
    RootInfo *root = new RootInfo(url, checkNeedCache(url));

    // insert it to rootInfoMap
    rootInfoMap.insert(url, root);

    return root;
}

bool FileDataManager::checkNeedCache(const QUrl &url)
{
    if (cacheDataSchemes.contains(url.scheme()))
        return true;

    // mounted dir should cache files in FileDataManager
    if (!FileUtils::isLocalDevice(url)) {
        qInfo() << "save data " << url;
        return true;
    }

    return false;
}
