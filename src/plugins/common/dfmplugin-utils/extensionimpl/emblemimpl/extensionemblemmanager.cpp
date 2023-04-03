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
    const QIcon &icon { QIcon::fromTheme(path) };
    if (!icon.name().isEmpty())
        return icon;

    return QIcon(path);
}

void EmblemIconWorker::onFetchEmblemIcons(const QList<QPair<QString, int>> &localPaths)
{
    Q_ASSERT(qApp->thread() != QThread::currentThread());
    if (localPaths.isEmpty())
        return;

    const auto &emblemPlugins = ExtensionPluginManager::instance().emblemPlugins();
    std::for_each(emblemPlugins.begin(), emblemPlugins.end(), [&localPaths, this](QSharedPointer<DFMEXT::DFMExtEmblemIconPlugin> plugin) {
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
}

bool EmblemIconWorker::parseLocationEmblemIcons(const QString &path, int count, QSharedPointer<dfmext::DFMExtEmblemIconPlugin> plugin)
{
    const auto &emblem { plugin->locationEmblemIcons(path.toStdString(), count) };
    const std::vector<DFMEXT::DFMExtEmblemIconLayout> &layouts { emblem.emblems() };
    if (layouts.empty())
        return false;

    if (embelmCaches.contains(path)) {   // check changed
        const QList<QPair<QString, int>> &oldGroup { embelmCaches[path] };
        QList<QPair<QString, int>> newGroup;
        makeLayoutGroup(layouts, &newGroup);
        QList<QPair<QString, int>> mergedGroup;
        mergeGroup(oldGroup, newGroup, &mergedGroup);
        if (mergedGroup != oldGroup) {
            embelmCaches[path] = mergedGroup;
            emit emblemIconChanged(path, mergedGroup);
            return true;
        }
    } else {   // save to cache
        QList<QPair<QString, int>> group;
        makeLayoutGroup(layouts, &group);
        emit emblemIconChanged(path, group);
        embelmCaches.insert(path, group);
        return true;
    }

    return false;
}

void EmblemIconWorker::parseEmblemIcons(const QString &path, int count, QSharedPointer<dfmext::DFMExtEmblemIconPlugin> plugin)
{
    const std::vector<std::string> &icons { plugin->emblemIcons(path.toStdString()) };

    if (icons.empty())
        return;
    if (icons.size() == 1 && icons.at(0).empty())
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

    *group = oldGroup;
    if (oldGroup.size() >= kMaxEmblemCount)
        return;

    // other plugins canot override first plugin's emblem icon
    QList<int> posSpace { 0, 1, 2, 3 };
    std::for_each(oldGroup.begin(), oldGroup.end(), [&posSpace](const QPair<QString, int> &pair) {
        posSpace.removeOne(pair.second);
    });

    std::for_each(newGroup.begin(), newGroup.end(), [posSpace, group](const QPair<QString, int> &pair) {
        if (posSpace.contains(pair.second))
            group->push_back(pair);
    });
}

ExtensionEmblemManager &ExtensionEmblemManager::instance()
{
    static ExtensionEmblemManager ins;
    return ins;
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
            qDebug() << "Not enough space paint emblem icon for extension lib url: " << url;
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
                    qWarning() << "Not position: " << pos << " to " << url;
                    continue;
                }
                const QString &iconName { group[i].first };
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

ExtensionEmblemManager::~ExtensionEmblemManager()
{
    Q_D(ExtensionEmblemManager);

    d->workerThread.quit();
    d->workerThread.wait();
}

DPUTILS_END_NAMESPACE
