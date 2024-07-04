// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <dfm-framework/dfm_framework_global.h>
#include <dfm-framework/lifecycle/plugin.h>
#include <dfm-framework/lifecycle/pluginmetaobject.h>

#include <QPluginLoader>
#include <QSettings>
#include <QObject>
#include <QQueue>
#include <QSharedData>
#include <QDirIterator>
#include <QScopedPointer>

DPF_BEGIN_NAMESPACE

class PluginManagerPrivate;

class PluginManager : public QObject
{
    Q_OBJECT
    friend class PluginManagerPrivate;
    QSharedPointer<PluginManagerPrivate> d;

public:
    explicit PluginManager();
    QStringList pluginIIDs() const;
    QStringList pluginPaths() const;
    QStringList blackList() const;
    QStringList lazyLoadList() const;
    QQueue<PluginMetaObjectPointer> readQueue() const;
    QQueue<PluginMetaObjectPointer> loadQueue() const;
    void addPluginIID(const QString &pluginIIDs);
    void addBlackPluginName(const QString &name);
    void addLazyLoadPluginName(const QString &name);
    void setPluginPaths(const QStringList &pluginPaths);
    void setLazyLoadFilter(std::function<bool(const QString &)> filter);
    void setBlackListFilter(std::function<bool(const QString &)> filter);

    bool readPlugins();
    bool loadPlugins();
    bool initPlugins();
    bool startPlugins();
    void stopPlugins();

    PluginMetaObjectPointer pluginMetaObj(const QString &pluginName,
                                          const QString version = "") const;

    bool loadPlugin(PluginMetaObjectPointer &pointer);
    bool initPlugin(PluginMetaObjectPointer &pointer);
    bool startPlugin(PluginMetaObjectPointer &pointer);
    bool stopPlugin(PluginMetaObjectPointer &pointer);
    bool isAllPluginsInitialized();
    bool isAllPluginsStarted();
};

DPF_END_NAMESPACE

#endif   // PLUGINMANAGER_H
