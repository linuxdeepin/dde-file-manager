// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "extensionemblemmanager_p.h"

#include <dfm-base/dfm_event_defines.h>

#include <dfm-framework/event/event.h>

#include <QCoreApplication>
#include <QDebug>
#include <QUrl>
#include <QIcon>
#include <QDir>

DPUTILS_BEGIN_NAMESPACE
DFMBASE_USE_NAMESPACE

static constexpr int kMaxEmblemCount { 4 };
static constexpr int kRequestReadyPathsTimeInterval { 500 };

ExtensionEmblemManagerPrivate::ExtensionEmblemManagerPrivate(ExtensionEmblemManager *qq)
    : q_ptr(qq)
{
}

ExtensionEmblemManagerPrivate::~ExtensionEmblemManagerPrivate()
{
}

void ExtensionEmblemManagerPrivate::addReadyLocalPath(const QPair<QString, int> &path)
{
    if (!readyLocalPaths.contains(path)) {
        readyLocalPaths.push_back(path);
        readyFlag = true;
    }
}

void ExtensionEmblemManagerPrivate::clearReadyLocalPath()
{
    readyLocalPaths.clear();
    readyFlag = false;
}

QIcon ExtensionEmblemManagerPrivate::makeIcon(const QString &path)
{
    if (path.isEmpty()) {
        fmDebug() << "Empty icon path provided";
        return QIcon();
    }

    QString cleanPath = QDir::cleanPath(path);

    auto it = iconCaches.constFind(cleanPath);
    if (it != iconCaches.constEnd())
        return *it;

    // 3. 尝试从主题加载
    auto icon = QIcon::fromTheme(cleanPath);
    if (!icon.isNull()) {
        // 找到主题图标，存入缓存并返回
        iconCaches.insert(cleanPath, icon);
        return icon;
    }

    // 4. 尝试作为文件路径加载
    // 这里进行安全检查
    if (QDir::isAbsolutePath(cleanPath)) {
        // 检查文件是否存在，防止无效路径反复尝试加载导致性能损耗
        if (QFile::exists(cleanPath)) {
            icon = QIcon(cleanPath);
            if (!icon.isNull()) {
                iconCaches.insert(cleanPath, icon); // 使用原始 path 作为键，或者 cleanPath
                return icon;
            }
        } else {
             fmWarning() << "Icon file does not exist:" << cleanPath;
        }
    } else {
        // 如果不是绝对路径，且 fromTheme 失败，可能是相对路径或无效路径
        // 根据业务需求，这里可以决定是否尝试相对路径加载
        // 出于安全考虑，通常不建议加载相对路径图标，除非有特定的基准目录
        fmWarning() << "Icon path is not a valid theme name or absolute path:" << path << ", cleanPath:" << cleanPath;
    }

    return QIcon();
}

void EmblemIconWorker::onFetchEmblemIcons(const QList<QPair<QString, int>> &localPaths)
{
    Q_ASSERT(qApp->thread() != QThread::currentThread());
    if (localPaths.isEmpty())
        return;

    const auto &emblemPlugins = ExtensionPluginManager::instance().emblemPlugins();
    std::for_each(emblemPlugins.begin(), emblemPlugins.end(), [&localPaths, this](DFMEXT::DFMExtEmblemIconPlugin *plugin) {
        Q_ASSERT(plugin);
        for (const auto &path : localPaths) {
            if (this->parseLocationEmblemIcons(path.first, path.second, plugin))
                continue;
            parseEmblemIcons(path.first, path.second, plugin);
        }
    });
}

void EmblemIconWorker::onClearCache()
{
    embelmCaches.clear();
    pluginCaches.clear();
}

bool EmblemIconWorker::parseLocationEmblemIcons(const QString &path, int count, dfmext::DFMExtEmblemIconPlugin *plugin)
{
    const auto &emblem { plugin->locationEmblemIcons(path.toStdString(), count) };
    const std::vector<DFMEXT::DFMExtEmblemIconLayout> &layouts { emblem.emblems() };
    // why add `pluginCaches` ?
    // To clear the emblem icon when a plugin returns an empty `DFMExtEmblemIconLayout`.
    quint64 pluginAddr { reinterpret_cast<quint64>(plugin) };
    const CacheType &curPluginCache { pluginCaches.value(pluginAddr) };
    if (layouts.empty() && curPluginCache.value(path).isEmpty())
        return false;

    if (embelmCaches.contains(path)) {   // check changed
        const QList<QPair<QString, int>> &oldGroup { embelmCaches[path] };
        QList<QPair<QString, int>> newGroup;
        makeLayoutGroup(layouts, &newGroup);
        newGroup = updateLayoutGroup(curPluginCache.value(path), newGroup);
        QList<QPair<QString, int>> mergedGroup;
        mergeGroup(oldGroup, newGroup, &mergedGroup);
        if (mergedGroup != oldGroup) {
            embelmCaches[path] = mergedGroup;
            saveToPluginCache(pluginAddr, path, newGroup);
            emit emblemIconChanged(path, mergedGroup);
        }
    } else {   // save to cache
        QList<QPair<QString, int>> group;
        makeLayoutGroup(layouts, &group);
        emit emblemIconChanged(path, group);
        embelmCaches.insert(path, group);
        saveToPluginCache(pluginAddr, path, group);
    }

    return true;
}

void EmblemIconWorker::parseEmblemIcons(const QString &path, int count, dfmext::DFMExtEmblemIconPlugin *plugin)
{
    quint64 pluginAddr { reinterpret_cast<quint64>(plugin) };
    if (hasCachedByOtherLocationEmblem(path, pluginAddr))
        return;
    const std::vector<std::string> &icons { plugin->emblemIcons(path.toStdString()) };

    if (icons.empty())
        return;

    if (embelmCaches.contains(path)) {   // check changed
        const QList<QPair<QString, int>> &oldGroup { embelmCaches[path] };
        QList<QPair<QString, int>> newGroup;
        makeNormalGroup(icons, count, &newGroup);
        QList<QPair<QString, int>> mergedGroup;
        mergeGroup(oldGroup, newGroup, &mergedGroup);
        if (mergedGroup != oldGroup) {
            embelmCaches[path] = mergedGroup;
            emit emblemIconChanged(path, mergedGroup);
        }
    } else {   // save to cache
        QList<QPair<QString, int>> group;
        makeNormalGroup(icons, count, &group);
        embelmCaches.insert(path, group);
        emit emblemIconChanged(path, group);
    }
}

EmblemIconWorker::CacheType EmblemIconWorker::makeCache(const QString &path, const QList<QPair<QString, int>> &group)
{
    CacheType cache;
    cache.insert(path, group);
    return cache;
}

void EmblemIconWorker::makeLayoutGroup(const std::vector<dfmext::DFMExtEmblemIconLayout> &layouts, QList<QPair<QString, int>> *group)
{
    Q_ASSERT(group);

    std::for_each(layouts.cbegin(), layouts.cend(), [group](const DFMEXT::DFMExtEmblemIconLayout &layout) {
        const QString &path { layout.iconPath().c_str() };
        int pos { int(layout.locationType()) };
        if (pos < kMaxEmblemCount)
            group->push_back({ path, pos });
    });
}

QList<QPair<QString, int>> EmblemIconWorker::updateLayoutGroup(const QList<QPair<QString, int>> &cache, const QList<QPair<QString, int>> &group)
{
    if (cache == group || cache.isEmpty())
        return group;

    QHash<int, QString> tmpMap;
    std::for_each(group.begin(), group.end(), [&tmpMap](const QPair<QString, int> &pair) {
        tmpMap.insert(pair.second, pair.first);
    });

    std::for_each(cache.begin(), cache.end(), [&tmpMap](const QPair<QString, int> &pair) {
        if (!tmpMap.contains(pair.second))
            tmpMap.insert(pair.second, "");
    });

    QList<QPair<QString, int>> updatedGroup;
    for (auto iter = tmpMap.begin(); iter != tmpMap.end(); ++iter) {
        if (updatedGroup.size() >= kMaxEmblemCount)
            break;

        updatedGroup.push_back(qMakePair(iter.value(), iter.key()));
    }

    return updatedGroup;
}

void EmblemIconWorker::makeNormalGroup(const std::vector<std::string> &icons, int count, QList<QPair<QString, int>> *group)
{
    Q_ASSERT(group);

    int index { 0 };
    std::for_each(icons.cbegin(), icons.cend(), [count, group, &index](const std::string &iconPath) {
        int pos { count + index };
        if (pos < kMaxEmblemCount) {
            group->push_back({ QString(iconPath.data()), pos });
            ++index;
        }
    });
}

void EmblemIconWorker::mergeGroup(const QList<QPair<QString, int>> &oldGroup,
                                  const QList<QPair<QString, int>> &newGroup,
                                  QList<QPair<QString, int>> *group)
{
    Q_ASSERT(group);

    // Merge old and new,
    // New overwrites old if same location exists

    QHash<int, QString> tmpMap;
    std::for_each(oldGroup.begin(), oldGroup.end(), [&tmpMap](const QPair<QString, int> &pair) {
        tmpMap.insert(pair.second, pair.first);
    });

    std::for_each(newGroup.begin(), newGroup.end(), [&tmpMap](const QPair<QString, int> &pair) {
        tmpMap.insert(pair.second, pair.first);
    });

    for (auto iter = tmpMap.begin(); iter != tmpMap.end(); ++iter) {
        if (group->size() >= kMaxEmblemCount)
            return;

        group->push_back(qMakePair(iter.value(), iter.key()));
    }
}

bool EmblemIconWorker::hasCachedByOtherLocationEmblem(const QString &path, quint64 addr)
{
    for (auto iter = pluginCaches.begin(); iter != pluginCaches.end(); ++iter) {
        const CacheType &cache { iter.value() };
        if (iter.key() != addr && cache.contains(path))
            return true;
    }
    return false;
}

void EmblemIconWorker::saveToPluginCache(quint64 addr, const QString &path, const QList<QPair<QString, int>> &group)
{
    if (pluginCaches.contains(addr)) {
        CacheType &cache = pluginCaches[addr];
        cache.insert(path, group);
        return;
    }

    pluginCaches.insert(addr, makeCache(path, group));
}

ExtensionEmblemManager &ExtensionEmblemManager::instance()
{
    static ExtensionEmblemManager ins;
    return ins;
}

void ExtensionEmblemManager::initialize()
{
    Q_D(ExtensionEmblemManager);

    qRegisterMetaType<QList<QPair<QString, int>>>();
    dpfSignalDispatcher->installEventFilter(GlobalEventType::kChangeCurrentUrl, this, &ExtensionEmblemManager::onUrlChanged);

    connect(&ExtensionPluginManager::instance(), &ExtensionPluginManager::allPluginsInitialized, this, &ExtensionEmblemManager::onAllPluginsInitialized);
    connect(&d->readyTimer, &QTimer::timeout, this, [this, d]() {
        if (d->readyFlag) {
            emit requestFetchEmblemIcon(d->readyLocalPaths);
            d->clearReadyLocalPath();   // for update
        }
    });
}

bool ExtensionEmblemManager::onFetchCustomEmblems(const QUrl &url, QList<QIcon> *emblems)
{
    Q_ASSERT(emblems);
    Q_ASSERT(qApp->thread() == QThread::currentThread());
    Q_D(ExtensionEmblemManager);

    if (Q_LIKELY(url.isValid())) {
        const QString &localPath { url.toLocalFile() };
        int currentCount { emblems->size() };
        // request load extension plugins
        if (ExtensionPluginManager::instance().currentState() != ExtensionPluginManager::kInitialized) {
            emit ExtensionPluginManager::instance().requestInitlaizePlugins();
            d->addReadyLocalPath({ localPath, currentCount });
            return false;
        }

        // no embelem icon extension
        if (!ExtensionPluginManager::instance().exists(ExtensionPluginManager::ExtensionType::kEmblemIcon))
            return false;

        // max emblem icon count
        if (currentCount > kMaxEmblemCount) {
            fmDebug() << "Not enough space paint emblem icon for extension lib url: " << url;
            return false;
        }

        // produce
        d->addReadyLocalPath({ localPath, currentCount });

        // consume
        if (d->positionEmbelmCaches.contains(localPath)) {
            const QList<QPair<QString, int>> &group { d->positionEmbelmCaches.value(localPath) };
            int lastSapce { kMaxEmblemCount - currentCount };
            // full
            for (int i = 0; i < lastSapce; ++i)
                emblems->push_back(QIcon());

            // repalce
            for (int i = 0; i < group.size(); ++i) {
                int pos { group[i].second };
                if (Q_UNLIKELY(pos >= kMaxEmblemCount))
                    continue;
                if (!emblems->at(pos).isNull()) {
                    fmWarning() << "Not position: " << pos << " to " << url;
                    continue;
                }
                const QString &iconName { group[i].first };
                if (iconName.isEmpty())
                    continue;
                emblems->replace(pos, d->makeIcon(iconName));
            }
            return true;
        }
    }

    return false;
}

void ExtensionEmblemManager::onEmblemIconChanged(const QString &path, const QList<QPair<QString, int>> &group)
{
    Q_D(ExtensionEmblemManager);
    d->positionEmbelmCaches[path] = group;
    auto eventID { DPF_NAMESPACE::Event::instance()->eventType("ddplugin_canvas", "slot_FileInfoModel_UpdateFile") };
    if (eventID != DPF_NAMESPACE::EventTypeScope::kInValid)
        dpfSlotChannel->push("ddplugin_canvas", "slot_FileInfoModel_UpdateFile", QUrl::fromLocalFile(path));
    else
        dpfSlotChannel->push("dfmplugin_workspace", "slot_Model_FileUpdate", QUrl::fromLocalFile(path));
}

void ExtensionEmblemManager::onAllPluginsInitialized()
{
    static std::once_flag flag;
    std::call_once(flag, [this]() {
        Q_D(ExtensionEmblemManager);

        EmblemIconWorker *worker { new EmblemIconWorker };
        worker->moveToThread(&d->workerThread);
        connect(&d->workerThread, &QThread::finished, worker, &QObject::deleteLater);
        connect(this, &ExtensionEmblemManager::requestFetchEmblemIcon, worker, &EmblemIconWorker::onFetchEmblemIcons);
        connect(this, &ExtensionEmblemManager::requestClearCache, worker, &EmblemIconWorker::onClearCache);
        connect(worker, &EmblemIconWorker::emblemIconChanged, this, &ExtensionEmblemManager::onEmblemIconChanged);

        d->workerThread.start();
        d->readyTimer.start(kRequestReadyPathsTimeInterval);
    });
}

bool ExtensionEmblemManager::onUrlChanged(quint64 windowId, const QUrl &url)
{
    Q_UNUSED(windowId);
    Q_UNUSED(url);
    Q_D(ExtensionEmblemManager);

    // clear all cache!
    d->clearReadyLocalPath();
    d->positionEmbelmCaches.clear();
    emit requestClearCache();

    return false;
}

ExtensionEmblemManager::ExtensionEmblemManager(QObject *parent)
    : QObject(parent), d_ptr(new ExtensionEmblemManagerPrivate(this))
{
}

ExtensionEmblemManager::~ExtensionEmblemManager()
{
    Q_D(ExtensionEmblemManager);

    // 停止定时器
    d->readyTimer.stop();
    
    // 确保工作线程正确退出
    if (d->workerThread.isRunning()) {
        d->workerThread.quit();
        if (!d->workerThread.wait(3000)) {
            fmWarning() << "ExtensionEmblemManager: Worker thread did not exit within 3 seconds, forcing termination";
            d->workerThread.terminate();
            d->workerThread.wait(1000);
        }
    }
}

DPUTILS_END_NAMESPACE
