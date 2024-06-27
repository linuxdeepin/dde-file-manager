// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "private/pluginquickmetadata_p.h"

#include <dfm-framework/lifecycle/pluginquickmetadata.h>

DPF_BEGIN_NAMESPACE

PluginQuickMetaData::PluginQuickMetaData()
    : d(new PluginQuickData)
{
}

/*!
 * \class PluginQuickMetaData
 * \brief 插件 Quick 组件元信息
 * \details 提供 Quikc 组件的基础信息
 *
 * \list
 *  \li url     QML 组件文件的路径，通过此信息加载组件
 *  \li id      组件ID
 *  \li plugin  插件ID
 *  \li type    组件类型，对应 Applet::Flag
 *  \li parent  当前组件的父组件，用于查找依赖关系，此字段以 [插件ID].[组件ID] 的格式填入
 *  \li applet  当前组件拓展 Applet 的工厂构造函数查找标识，用于查找创建插件注册的 Applet
 * \endlist
 */
PluginQuickMetaData::PluginQuickMetaData(
        const QUrl &url, const QString &id, const QString &plugin, const QString &type, const QString &parent, const QString &applet)
    : d(new PluginQuickData)
{
    d->quickUrl = url;
    d->quickId = id;
    d->plugin = plugin;
    d->quickType = type;
    d->quickParent = parent;
    d->quickApplet = applet;
}

QUrl PluginQuickMetaData::url() const
{
    return d->quickUrl;
}

QString PluginQuickMetaData::id() const
{
    return d->quickId;
}

QString PluginQuickMetaData::plugin() const
{
    return d->plugin;
}

QString PluginQuickMetaData::type() const
{
    return d->quickType;
}

QString PluginQuickMetaData::parent() const
{
    return d->quickParent;
}

QString PluginQuickMetaData::applet() const
{
    return d->quickApplet;
}

PluginQuickMetaDataCreator::PluginQuickMetaDataCreator() { }

void PluginQuickMetaDataCreator::create(const QString &plugin, const QString &id, const QUrl &url)
{
    metaPtr = PluginQuickMetaPtr::create();
    metaPtr->d->plugin = plugin;
    metaPtr->d->quickId = id;
    metaPtr->d->quickUrl = url;
}

void PluginQuickMetaDataCreator::setType(const QString &type)
{
    if (metaPtr) {
        metaPtr->d->quickType = type;
    }
}

void PluginQuickMetaDataCreator::setParent(const QString &parent)
{
    if (metaPtr) {
        metaPtr->d->quickParent = parent;
    }
}

void PluginQuickMetaDataCreator::setApplet(const QString &applet)
{
    if (metaPtr) {
        metaPtr->d->quickApplet = applet;
    }
}

PluginQuickMetaPtr PluginQuickMetaDataCreator::take()
{
    PluginQuickMetaPtr tempPtr(std::move(metaPtr));
    return tempPtr;
}

DPF_END_NAMESPACE
