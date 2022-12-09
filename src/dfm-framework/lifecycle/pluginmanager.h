/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
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
#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include "dfm-framework/dfm_framework_global.h"
#include "dfm-framework/lifecycle/plugin.h"
#include "dfm-framework/lifecycle/pluginmetaobject.h"

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
    void addPluginIID(const QString &pluginIIDs);
    void addBlackPluginName(const QString &name);
    void addLazyLoadPluginName(const QString &name);
    void setPluginPaths(const QStringList &pluginPaths);

    bool readPlugins();
    bool loadPlugins();
    void initPlugins();
    void startPlugins();
    void stopPlugins();

    PluginMetaObjectPointer pluginMetaObj(const QString &pluginName,
                                          const QString version = "") const;

    bool loadPlugin(PluginMetaObjectPointer &pointer);
    bool initPlugin(PluginMetaObjectPointer &pointer);
    bool startPlugin(PluginMetaObjectPointer &pointer);
    void stopPlugin(PluginMetaObjectPointer &pointer);
    bool isAllPluginsInitialized();
    bool isAllPluginsStarted();
};

DPF_END_NAMESPACE

#endif   // PLUGINMANAGER_H
