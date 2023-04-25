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
    QStringList blackPlguinNames;
    QStringList lazyLoadPluginsNames;
    QStringList loadedVirtualPlugins;
    QStringList unloadedVirtualPlugins;
    QQueue<PluginMetaObjectPointer> readQueue;
    QQueue<PluginMetaObjectPointer> notLazyLoadQuene;
    QQueue<PluginMetaObjectPointer> loadQueue;
    bool allPluginsInitialized { false };
    bool allPluginsStarted { false };

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

    static void scanfAllPlugin(QQueue<PluginMetaObjectPointer> *destQueue,
                               const QStringList &pluginPaths,
                               const QStringList &pluginIIDs,
                               const QStringList &blackList);
    static void scanfRealPlugin(QQueue<PluginMetaObjectPointer> *destQueue, PluginMetaObjectPointer metaObj,
                                const QJsonObject &dataJson, const QStringList &blackList);
    static void scanfVirtualPlugin(QQueue<PluginMetaObjectPointer> *destQueue, const QString &fileName,
                                   const QJsonObject &dataJson, const QStringList &blackList);
    static void readJsonToMeta(PluginMetaObjectPointer metaObject);
    static void jsonToMeta(PluginMetaObjectPointer metaObject, const QJsonObject &metaData);
    static void dependsSort(QQueue<PluginMetaObjectPointer> *dstQueue,
                            const QQueue<PluginMetaObjectPointer> *srcQueue);

private:
    bool doLoadPlugin(PluginMetaObjectPointer pointer);
    bool doInitPlugin(PluginMetaObjectPointer pointer);
    bool doStartPlugin(PluginMetaObjectPointer pointer);
    bool doStopPlugin(PluginMetaObjectPointer pointer);

    static bool doPluginSort(const PluginDependGroup group,
                             QMap<QString, PluginMetaObjectPointer> src,
                             QQueue<PluginMetaObjectPointer> *dest);
};

DPF_END_NAMESPACE

#endif   // PLUGINMANAGER_P_H
