// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LIFECYCLE_H
#define LIFECYCLE_H

#include <dfm-framework/lifecycle/pluginmetaobject.h>
#include <dfm-framework/lifecycle/plugin.h>
#include <dfm-framework/lifecycle/plugincreator.h>
#include <dfm-framework/dfm_framework_global.h>

#include <QString>
#include <QObject>

#include <functional>

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
QList<PluginMetaObjectPointer> pluginMetaObjs(const std::function<bool(PluginMetaObjectPointer)> &cond = {});
QList<PluginMetaObjectPointer> pluginSortedMetaObjs(const std::function<bool(PluginMetaObjectPointer)> &cond = {});

bool readPlugins();
bool loadPlugins();
void shutdownPlugins();

bool loadPlugin(PluginMetaObjectPointer &pointer);
void shutdownPlugin(PluginMetaObjectPointer &pointer);

void setLazyloadFilter(std::function<bool(const QString &)> filter);
void setBlackListFilter(std::function<bool(const QString &)> filter);
}   // namepsace LifeCycle

DPF_END_NAMESPACE

#endif   // LIFECYCLE_H
