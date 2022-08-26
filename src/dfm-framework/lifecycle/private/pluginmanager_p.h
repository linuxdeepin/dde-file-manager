/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef PLUGINMANAGER_P_H
#define PLUGINMANAGER_P_H

#include "dfm-framework/dfm_framework_global.h"
#include "dfm-framework/lifecycle/pluginmetaobject.h"

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
    QStringList loadedVirtualPlugins;
    QStringList unloadedVirtualPlugins;
    QQueue<PluginMetaObjectPointer> readQueue;
    QQueue<PluginMetaObjectPointer> loadQueue;

public:
    explicit PluginManagerPrivate(PluginManager *qq);
    virtual ~PluginManagerPrivate();

    PluginMetaObjectPointer pluginMetaObj(const QString &name, const QString &version = "");
    bool loadPlugin(PluginMetaObjectPointer &pluginMetaObj);
    bool initPlugin(PluginMetaObjectPointer &pluginMetaObj);
    bool startPlugin(PluginMetaObjectPointer &pluginMetaObj);
    void stopPlugin(PluginMetaObjectPointer &pluginMetaObj);

    bool readPlugins();
    bool loadPlugins();
    bool initPlugins();
    bool startPlugins();
    void stopPlugins();

    static QMutex *mutex();
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
    void doStopPlugin(PluginMetaObjectPointer pointer);

    static bool doPluginSort(const PluginDependGroup group,
                             QMap<QString, PluginMetaObjectPointer> src,
                             QQueue<PluginMetaObjectPointer> *dest);
};

DPF_END_NAMESPACE

#endif   // PLUGINMANAGER_P_H
