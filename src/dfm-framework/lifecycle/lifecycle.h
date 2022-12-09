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
#ifndef LIFECYCLE_H
#define LIFECYCLE_H

#include "dfm-framework/lifecycle/pluginmetaobject.h"
#include "dfm-framework/lifecycle/plugin.h"
#include "dfm-framework/lifecycle/plugincreator.h"
#include "dfm-framework/dfm_framework_global.h"

#include <QString>
#include <QObject>

DPF_BEGIN_NAMESPACE

namespace LifeCycle {
void initialize(const QStringList &IIDs, const QStringList &paths);
void initialize(const QStringList &IIDs, const QStringList &paths, const QStringList &blackNames);
void initialize(const QStringList &IIDs, const QStringList &paths, const QStringList &blackNames,
                const QStringList &lazyNames);

bool isAllPluginsInitialized();
bool isAllPluginsStarted();
QStringList pluginIIDs();
QStringList pluginPaths();
QStringList blackList();
QStringList lazyLoadList();
PluginMetaObjectPointer pluginMetaObj(const QString &pluginName,
                                      const QString version = "");

bool readPlugins();
bool loadPlugins();
void shutdownPlugins();

bool loadPlugin(PluginMetaObjectPointer &pointer);
void shutdownPlugin(PluginMetaObjectPointer &pointer);
}   // namepsace LifeCycle

DPF_END_NAMESPACE

#endif   // LIFECYCLE_H
