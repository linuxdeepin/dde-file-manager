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
#ifndef PLUGINMETAOBJECT_P_H
#define PLUGINMETAOBJECT_P_H

#include "dfm-framework/dfm_framework_global.h"
#include "dfm-framework/lifecycle/pluginmetaobject.h"

#include <QString>
#include <QStringList>
#include <QSharedPointer>

DPF_BEGIN_NAMESPACE

/// \brief kPluginName 插件名称Key
const char kPluginName[] = "Name";
/// \brief kPluginVersion 插件版本Key
const char kPluginVersion[] = "Version";
/// \brief kPluginCompatversion 插件兼容版本Key
const char kPluginCompatversion[] = "CompatVersion";
/// \brief kPluginCategory 插件类型Key
const char kPluginCategory[] = "Category";
/// \nrief kPluginVendor 插件作者
const char kPluginVendor[] = "Vendor";
/// \nrief kPluginCopyright 插件所持有的公司
const char kPluginCopyright[] = "Copyright";
/// \nrief kPluginDescription 插件描述
const char kPluginDescription[] = "Description";
/// \nrief kPluginLicense 插件开源协议
const char kPluginLicense[] = "License";
/// \nrief kPluginUrllink 插件主页链接地址
const char kPluginUrlLink[] = "UrlLink";
/// \nrief kPluginDepends 插件依赖
const char kPluginDepends[] = "Depends";

class PluginMetaObject;
class PluginMetaObjectPrivate
{
    friend class PluginManager;
    friend class PluginManagerPrivate;
    friend class PluginMetaObject;
    PluginMetaObject const *q;

public:
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
    QSharedPointer<PluginContext> context;

    explicit PluginMetaObjectPrivate(PluginMetaObject *q)
        : q(q), loader(new QPluginLoader(nullptr))
    {
    }
};

DPF_END_NAMESPACE

#endif   // PLUGINMETAOBJECT_P_H
