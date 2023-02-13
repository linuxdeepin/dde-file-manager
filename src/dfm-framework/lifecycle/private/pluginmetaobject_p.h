#ifndef PLUGINMETAOBJECT_P_H
#define PLUGINMETAOBJECT_P_H

#include <dfm-framework/dfm_framework_global.h>
#include <dfm-framework/lifecycle/pluginmetaobject.h>
#include <dfm-framework/lifecycle/plugincreator.h>

#include <QString>
#include <QStringList>
#include <QSharedPointer>

DPF_BEGIN_NAMESPACE

/// \brief kPluginName 插件名称Key
inline constexpr char kPluginName[] { "Name" };
/// \brief kPluginVersion 插件版本Key
inline constexpr char kPluginVersion[] { "Version" };
/// \brief kPluginCompatversion 插件兼容版本Key
inline constexpr char kPluginCompatversion[] { "CompatVersion" };
/// \brief kPluginCategory 插件类型Key
inline constexpr char kPluginCategory[] { "Category" };
/// \brief kPluginVendor 插件作者
inline constexpr char kPluginVendor[] { "Vendor" };

// Copyright 插件所持有的公司
// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

inline constexpr char kPluginCopyright[] { "Copyright" };
/// \brief kPluginDescription 插件描述
inline constexpr char kPluginDescription[] { "Description" };
/// \brief kPluginLicense 插件开源协议
inline constexpr char kPluginLicense[] { "License" };
/// \brief kPluginUrllink 插件主页链接地址
inline constexpr char kPluginUrlLink[] { "UrlLink" };
/// \brief kPluginDepends 插件依赖
inline constexpr char kPluginDepends[] { "Depends" };
/// \brief kPluginDepends virtual plugin meta info
inline constexpr char kVirtualPluginMeta[] { "Meta" };
/// \brief kPluginDepends virtual plugin info list
inline constexpr char kVirtualPluginList[] { "VirtualPlugins" };

class PluginMetaObject;
class PluginMetaObjectPrivate
{
    friend class PluginManager;
    friend class PluginManagerPrivate;
    friend class PluginMetaObject;
    PluginMetaObject const *q;

public:
    bool isVirtual { false };
    QString realName;   // only virtual plugin

    QString iid;
    QString name;
    QString version;
    QString compatVersion;
    QString vendor;
    QString copyright;
    QStringList license;
    QString description;
    QString urlLink;
    QString category;
    QString error;
    PluginMetaObject::State state;
    QList<PluginDepend> depends;
    QSharedPointer<Plugin> plugin;
    QSharedPointer<QPluginLoader> loader;

    explicit PluginMetaObjectPrivate(PluginMetaObject *q)
        : q(q), loader(new QPluginLoader(nullptr))
    {
    }
};

DPF_END_NAMESPACE

#endif   // PLUGINMETAOBJECT_P_H
