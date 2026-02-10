// SPDX-FileCopyrightText: 2020 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "private/pluginmanager_p.h"

#include <dfm-framework/lifecycle/pluginmanager.h>

DPF_USE_NAMESPACE

PluginManager::PluginManager()
    : d(new PluginManagerPrivate(this))
{
    qCDebug(logDPF) << "PluginManager: instance created";
}

void PluginManager::addPluginIID(const QString &pluginIID)
{
    if (d->pluginLoadIIDs.contains(pluginIID)) {
        qCDebug(logDPF) << "PluginManager: IID already exists:" << pluginIID;
        return;
    }
    d->pluginLoadIIDs.push_back(pluginIID);
    qCDebug(logDPF) << "PluginManager: added IID:" << pluginIID;
}

void PluginManager::addBlackPluginName(const QString &name)
{
    if (!d->blackPluginNames.contains(name)) {
        d->blackPluginNames.push_back(name);
        qCDebug(logDPF) << "PluginManager: added to blacklist:" << name;
    } else {
        qCDebug(logDPF) << "PluginManager: plugin already in blacklist:" << name;
    }
}

void PluginManager::addLazyLoadPluginName(const QString &name)
{
    if (!d->lazyLoadPluginNames.contains(name)) {
        d->lazyLoadPluginNames.push_back(name);
        qCDebug(logDPF) << "PluginManager: added to lazy load list:" << name;
    } else {
        qCDebug(logDPF) << "PluginManager: plugin already in lazy load list:" << name;
    }
}

void PluginManager::setQtVersionInsensitivePluginNames(const QStringList &names)
{
    d->qtVersionInsensitivePluginNames = names;
    qCInfo(logDPF) << "PluginManager: set" << names.size() << "Qt version insensitive plugins";
}

void PluginManager::setPluginPaths(const QStringList &pluginPaths)
{
    d->pluginLoadPaths = pluginPaths;
    qCInfo(logDPF) << "PluginManager: set" << pluginPaths.size() << "plugin paths";
    qCDebug(logDPF) << "PluginManager: plugin paths:" << pluginPaths;
}

void PluginManager::setLazyLoadFilter(std::function<bool(const QString &)> filter)
{
    d->lazyPluginFilter = filter;
    qCDebug(logDPF) << "PluginManager: lazy load filter set";
}

void PluginManager::setBlackListFilter(std::function<bool(const QString &)> filter)
{
    d->blackListFilter = filter;
    qCDebug(logDPF) << "PluginManager: blacklist filter set";
}

PluginMetaObjectPointer PluginManager::pluginMetaObj(const QString &pluginName, const QString version) const
{
    Q_UNUSED(version)
    PluginMetaObjectPointer result = d->pluginMetaObj(pluginName);
    if (result) {
        qCDebug(logDPF) << "PluginManager: found plugin meta object:" << pluginName;
    } else {
        qCDebug(logDPF) << "PluginManager: plugin meta object not found:" << pluginName;
    }
    return result;
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
    return d->lazyLoadPluginNames;
}

QQueue<PluginMetaObjectPointer> PluginManager::readQueue() const
{
    return d->readQueue;
}

QQueue<PluginMetaObjectPointer> PluginManager::loadQueue() const
{
    return d->loadQueue;
}
