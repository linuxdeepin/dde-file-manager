// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PLUGINMETAOBJECT_P_H
#define PLUGINMETAOBJECT_P_H

#include <dfm-framework/dfm_framework_global.h>
#include <dfm-framework/lifecycle/pluginmetaobject.h>
#include <dfm-framework/lifecycle/plugincreator.h>
#include <dfm-framework/lifecycle/pluginquickmetadata.h>

#include <QString>
#include <QStringList>
#include <QSharedPointer>
#include <QVariantMap>

DPF_BEGIN_NAMESPACE

/// \brief kPluginName 插件名称Key
inline constexpr char kPluginName[] { "Name" };
/// \brief kPluginVersion 插件版本Key
inline constexpr char kPluginVersion[] { "Version" };
/// \brief kPluginCategory 插件类型Key
inline constexpr char kPluginCategory[] { "Category" };
/// \brief kPluginDescription 插件描述
inline constexpr char kPluginDescription[] { "Description" };
/// \brief kPluginUrllink 插件主页链接地址
inline constexpr char kPluginUrlLink[] { "UrlLink" };
/// \brief kPluginDepends 插件依赖
inline constexpr char kPluginDepends[] { "Depends" };
/// \brief kCustomData 插件自定义数据
inline constexpr char kCustomData[] { "Custom" };
/// \brief kPluginDepends virtual plugin meta info
inline constexpr char kVirtualPluginMeta[] { "Meta" };
/// \brief kPluginDepends virtual plugin info list
inline constexpr char kVirtualPluginList[] { "VirtualPlugins" };
/// \brief kQuickUrl QML组件信息
inline constexpr char kQuick[] { "Quick" };
/// \brief kQuickUrl QML组件文件Url
inline constexpr char kQuickUrl[] { "Url" };
/// \brief kQuickUrl QML组件文件ID
inline constexpr char kQuickId[] { "Id" };
/// \brief kQuickUrl QML组件类型，可选 Containment | Window 空内容视为 Applet
inline constexpr char kQuickType[] { "Type" };
/// \brief kQuickUrl QML组件父组件名称(使用插件Name)，必须在 "Depends" 字段中存在
inline constexpr char kQuickParent[] { "Parent" };
/// \brief kQuickUrl QML组件拓展Applet 需在 SchemeFactory 注册
inline constexpr char kQuickApplet[] { "Applet" };

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
    QString description;
    QString urlLink;
    QString category;
    QString error;
    PluginMetaObject::State state { PluginMetaObject::kInvalid };
    QList<PluginDepend> depends;
    QSharedPointer<Plugin> plugin;
    QSharedPointer<QPluginLoader> loader;
    QVariantMap customData;
    QList<PluginQuickMetaPtr> quickMetaList;

    explicit PluginMetaObjectPrivate(PluginMetaObject *q)
        : q(q), loader(new QPluginLoader(nullptr))
    {
    }
};

DPF_END_NAMESPACE

#endif   // PLUGINMETAOBJECT_P_H
