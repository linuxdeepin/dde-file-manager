// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PLUGINMANAGER_P_H
#define PLUGINMANAGER_P_H

#include <dfm-framework/dfm_framework_global.h>
#include <dfm-framework/lifecycle/pluginmetaobject.h>

#include <QQueue>
#include <QStringList>
#include <QPluginLoader>
#include <QObject>
#include <QJsonArray>
#include <QSettings>
#include <QDirIterator>
#include <QDebug>
#include <QWriteLocker>
#include <QtConcurrent>

DPF_BEGIN_NAMESPACE

class PluginMetaObject;
class PluginManager;

class PluginManagerPrivate : public QSharedData
{
    Q_DISABLE_COPY(PluginManagerPrivate)
    friend class PluginManager;
    PluginManager *const q;
    QStringList pluginLoadIIDs;
    QStringList pluginLoadPaths;
    QStringList blackPluginNames;
    QStringList lazyLoadPluginsNames;
    QStringList loadedVirtualPlugins;
    QStringList unloadedVirtualPlugins;
    QQueue<PluginMetaObjectPointer> readQueue;
    QQueue<PluginMetaObjectPointer> pluginsToLoad;
    QQueue<PluginMetaObjectPointer> loadQueue;
    bool allPluginsInitialized { false };
    bool allPluginsStarted { false };
    std::function<bool(const QString &)> lazyPluginFilter;
    std::function<bool(const QString &)> blackListFilter;

public:
    explicit PluginManagerPrivate(PluginManager *qq);
    virtual ~PluginManagerPrivate();

    PluginMetaObjectPointer pluginMetaObj(const QString &name);
    bool loadPlugin(PluginMetaObjectPointer &pluginMetaObj);
    bool initPlugin(PluginMetaObjectPointer &pluginMetaObj);
    bool startPlugin(PluginMetaObjectPointer &pluginMetaObj);
    bool stopPlugin(PluginMetaObjectPointer &pluginMetaObj);

    bool readPlugins();
    bool loadPlugins();
    bool initPlugins();
    bool startPlugins();
    void stopPlugins();

private:
    bool doLoadPlugin(PluginMetaObjectPointer pointer);
    bool doInitPlugin(PluginMetaObjectPointer pointer);
    bool doStartPlugin(PluginMetaObjectPointer pointer);
    bool doStopPlugin(PluginMetaObjectPointer pointer);

    void scanfAllPlugin();
    void scanfRealPlugin(PluginMetaObjectPointer metaObj,
                         const QJsonObject &dataJson);
    void scanfVirtualPlugin(const QString &fileName,
                            const QJsonObject &dataJson);
    bool isBlackListed(const QString &name);

    void readJsonToMeta(PluginMetaObjectPointer metaObject);
    void jsonToMeta(PluginMetaObjectPointer metaObject, const QJsonObject &metaData);
    void dependsSort(QQueue<PluginMetaObjectPointer> *dstQueue,
                     const QQueue<PluginMetaObjectPointer> *srcQueue);
    bool doPluginSort(const PluginDependGroup group,
                      QMap<QString, PluginMetaObjectPointer> src,
                      QQueue<PluginMetaObjectPointer> *dest);
};

DPF_END_NAMESPACE

#endif   // PLUGINMANAGER_P_H
