// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "private/pluginmanager_p.h"

#include <dfm-framework/lifecycle/pluginmanager.h>

DPF_USE_NAMESPACE

PluginManager::PluginManager()
    : d(new PluginManagerPrivate(this))
{
}

void PluginManager::addPluginIID(const QString &pluginIID)
{
    if (d->pluginLoadIIDs.contains(pluginIID))
        return;
    d->pluginLoadIIDs.push_back(pluginIID);
}

void PluginManager::addBlackPluginName(const QString &name)
{
    if (!d->blackPluginNames.contains(name))
        d->blackPluginNames.push_back(name);
}

void PluginManager::addLazyLoadPluginName(const QString &name)
{
    if (!d->lazyLoadPluginsNames.contains(name))
        d->lazyLoadPluginsNames.push_back(name);
}

void PluginManager::setPluginPaths(const QStringList &pluginPaths)
{
    d->pluginLoadPaths = pluginPaths;
}

void PluginManager::setLazyLoadFilter(std::function<bool(const QString &)> filter)
{
    d->lazyPluginFilter = filter;
}

void PluginManager::setBlackListFilter(std::function<bool(const QString &)> filter)
{
    d->blackListFilter = filter;
}

PluginMetaObjectPointer PluginManager::pluginMetaObj(const QString &pluginName, const QString version) const
{
    Q_UNUSED(version)
    return d->pluginMetaObj(pluginName);
}

bool PluginManager::loadPlugin(PluginMetaObjectPointer &pointer)
{
    return d->loadPlugin(pointer);
}

bool PluginManager::initPlugin(PluginMetaObjectPointer &pointer)
{
    return d->initPlugin(pointer);
}

bool PluginManager::startPlugin(PluginMetaObjectPointer &pointer)
{
    return d->startPlugin(pointer);
}

bool PluginManager::stopPlugin(PluginMetaObjectPointer &pointer)
{
    return d->stopPlugin(pointer);
}

bool PluginManager::isAllPluginsInitialized()
{
    return d->allPluginsInitialized;
}

bool PluginManager::isAllPluginsStarted()
{
    return d->allPluginsStarted;
}

bool PluginManager::readPlugins()
{
    return d->readPlugins();
}

bool PluginManager::loadPlugins()
{
    return d->loadPlugins();
}

bool PluginManager::initPlugins()
{
    return d->initPlugins();
}

bool PluginManager::startPlugins()
{
    return d->startPlugins();
}

void PluginManager::stopPlugins()
{
    d->stopPlugins();
}

QStringList PluginManager::pluginIIDs() const
{
    return d->pluginLoadIIDs;
}

QStringList PluginManager::pluginPaths() const
{
    return d->pluginLoadPaths;
}

QStringList PluginManager::blackList() const
{
    return d->blackPluginNames;
}

QStringList PluginManager::lazyLoadList() const
{
    return d->lazyLoadPluginsNames;
}

QQueue<PluginMetaObjectPointer> PluginManager::readQueue() const
{
    return d->readQueue;
}

QQueue<PluginMetaObjectPointer> PluginManager::loadQueue() const
{
    return d->loadQueue;
}
