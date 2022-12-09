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
#include "private/pluginmanager_p.h"
#include "pluginmanager.h"

DPF_USE_NAMESPACE

/*!
 * \class PluginManager 插件管理器
 * \details 提供插件加载与卸载
 * 其中重要的特性为：plugin IID (插件身份标识) 可参阅Qt插件规范；
 * 此处目前只支持Plugin接口插件的动态库形式插件加载
 */

/*! \brief PluginManager的构造函数
 */
PluginManager::PluginManager()
    : d(new PluginManagerPrivate(this))
{
}

/*!
 * \brief Add a class of plug-in IID to the framework,
 * the framework will load the IID plug-in to achieve an
 * APP can load a variety of different IID plug-in capabilities
 * \param pluginIID: like org.deepin.plugin.[XXX]
 */
void PluginManager::addPluginIID(const QString &pluginIID)
{
    if (d->pluginLoadIIDs.contains(pluginIID))
        return;
    d->pluginLoadIIDs.push_back(pluginIID);
}

void PluginManager::addBlackPluginName(const QString &name)
{
    if (!d->blackPlguinNames.contains(name))
        d->blackPlguinNames.push_back(name);
}

void PluginManager::addLazyLoadPluginName(const QString &name)
{
    if (!d->lazyLoadPluginsNames.contains(name))
        d->lazyLoadPluginsNames.push_back(name);
}

/*!
 * \brief setPluginPaths 设置插件加载的路径
 * \param const QStringList &pluginPaths 传入路径列表
 * \return void
 */
void PluginManager::setPluginPaths(const QStringList &pluginPaths)
{
    d->pluginLoadPaths = pluginPaths;
}

PluginMetaObjectPointer PluginManager::pluginMetaObj(const QString &pluginName, const QString version) const
{
    return d->pluginMetaObj(pluginName, version);
}

bool PluginManager::loadPlugin(PluginMetaObjectPointer &pointer)
{
    return d->loadPlugin(pointer);
}

bool PluginManager::initPlugin(PluginMetaObjectPointer &pointer)
{
    return d->initPlugin(pointer);
}

bool PluginManager::startPlugin(PluginMetaObjectPointer &pointer)
{
    return d->startPlugin(pointer);
}

void PluginManager::stopPlugin(PluginMetaObjectPointer &pointer)
{
    return d->stopPlugin(pointer);
}

bool PluginManager::isAllPluginsInitialized()
{
    return d->allPluginsInitialized;
}

bool PluginManager::isAllPluginsStarted()
{
    return d->allPluginsStarted;
}

/*!
 * \brief readPlugins 读取插件元数据
 * \details [此函数是线程安全的]
 *  读取当前设置插件路径下的所有插件元数据
 * \return void
 */
bool PluginManager::readPlugins()
{
    return d->readPlugins();
}

/*!
 * \brief loadPlugins 加载所有的插件
 * \pre 需要先执行readPlugins方法
 * \details [此函数是线程安全的]
 * 加载所有插件，内部使用Qt类QPluginLoader
 * \return void
 */
bool PluginManager::loadPlugins()
{
    return d->loadPlugins();
}

/*!
 * \brief initPlugins 执行所有插件initialized接口
 * \pre 需要先执行loadPlugins方法
 * \details [此函数是线程安全的]
 * 调用插件接口Plugin中initialized函数，
 * \return void
 */
void PluginManager::initPlugins()
{
    d->initPlugins();
}

/*!
 * \brief startPlugins 执行所有插件代码Plugin::start接口
 * \pre 执行initPlugins后插件执行的函数，
 * 否则可能遇到未知崩溃，
 * 因为该操作的存在是未定义的。
 * \details [此函数是线程安全的]
 * 调用插件接口Plugin中start函数
 * \return void
 */
void PluginManager::startPlugins()
{
    d->startPlugins();
}

/*!
 * \brief stopPlugins 执行所有插件代码Plugin::stop接口
 * \pre 执行startPlugins后插件执行的函数，
 * 否则可能遇到未知崩溃，
 * 因为该操作的存在是未定义的。
 * \details [此函数是线程安全的]
 * 调用插件接口Plugin中stop函数，并且卸载加载的插件。
 * \return void
 */
void PluginManager::stopPlugins()
{
    d->stopPlugins();
}

/*!
 * \brief pluginIID 获取插件标识IID
 * \return QString 返回设置的IID，默认为空
 */
QStringList PluginManager::pluginIIDs() const
{
    return d->pluginLoadIIDs;
}

/*!
 * \brief pluginPaths 获取插件路径
 * \return QString 返回设置的插件路径，默认为空
 */
QStringList PluginManager::pluginPaths() const
{
    return d->pluginLoadPaths;
}

QStringList PluginManager::blackList() const
{
    return d->blackPlguinNames;
}

QStringList PluginManager::lazyLoadList() const
{
    return d->lazyLoadPluginsNames;
}
