/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
 *             huangyu<zhangyub@uniontech.com>
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
#include "private/dfmextpluginmanager_p.h"
#include "dfmextpluginmanager.h"
#include "dfmstandardpaths.h"

#include <QDirIterator>
#include <QDebug>
#include <QLibrary>

/*!
 * \brief DFMExtPluginManager::pluginMenuProxy 获取插件菜单的代理
 * \return 栈上代理对象的指针
 */
DFMExtMenuImplProxy *DFMExtPluginManager::pluginMenuProxy()
{
    return &(d->menuImplProxy);
}

/*!
 * \brief setPluginPaths 设置插件路径
 * \param paths 插件路径列表形参
 */
void DFMExtPluginManager::setPluginPaths(const QStringList &paths)
{
    d->pluginPaths = paths;
}

/*!
 * \brief pluginPaths 获取当前设置的插件路径
 * \return 返回路径列表
 */
QStringList DFMExtPluginManager::pluginPaths() const
{
    return d->pluginPaths;
}

/*!
 * \brief scannPlugins 扫描已设置路径下的所有插件
 * \return 返回执行结果，如果扫描路径下存在不可失败的so文件将返回false
 */
bool DFMExtPluginManager::scannPlugins()
{
    for(auto val : d->pluginPaths) {
        QDirIterator itera(val, QDir::Files | QDir::NoSymLinks);
        while (itera.hasNext()) {
            itera.next();
            if (DFMExtPluginLoader::hasSymbol(itera.filePath())) {
                DFMExtPluginLoaderPointer autoPointer(new DFMExtPluginLoader(itera.filePath()));
                d->loaders.insert(itera.filePath(),autoPointer);
            }
        }
    }
    qDebug() << "loader import files name" << d->loaders.keys();
    d->currState = DFMExtPluginManager::State::Scanned;
    return true;
}

/*!
 * \brief loadPlugins 执行插件加载函数
 * \return 返回执行结果，如果任意插件执行该流程失败则返回false
 */
bool DFMExtPluginManager::loadPlugins()
{
    bool ret = true;
    for (auto val : d->loaders) {
        if (!val->loadPlugin()){
            ret = false;
            qInfo() << val->errorString();
        }
    }
    d->currState = DFMExtPluginManager::State::Loaded;
    return ret;
}

/*!
 * \brief initPlugins 执行插件初始化函数
 * \return 返回执行结果，如果任意插件执行该流程失败则返回false
 */
bool DFMExtPluginManager::initPlugins()
{
    bool ret = true;
    for (auto val : d->loaders) {
        if(!val->initialize()){
            ret = false;
            qInfo() << val->errorString();
        }
        d->menus.append(val->extMenuPlugin());
        d->emblemIcons.append(val->extEmbleIconPlugin());
    }
    d->currState = DFMExtPluginManager::State::Initialized;
    return ret;
}

/*!
 * \brief shutdownPlugins 回收所有插件内部资源并且卸载插件
 * \return 返回执行结果，如果任意插件执行该流程失败则返回false
 */
bool DFMExtPluginManager::shutdownPlugins()
{
    for (auto val : d->loaders) {
        val->shutdown();
    }
    d->currState = DFMExtPluginManager::State::Shutdown;
    return true;
}

/*!
 * \brief menus 获取插件内部右键菜单扩展接口
 * \return 返回接口列表
 */
DFMExtPluginManager::DFMExtMenus DFMExtPluginManager::menus()
{
    return d->menus;
}

/*!
 * \brief emblemIcons 获取插件内部角标扩展接口
 * \return 返回接口列表
 */
DFMExtPluginManager::DFMExtEmblemIcons DFMExtPluginManager::emblemIcons()
{
    return d->emblemIcons;
}

/*!
 * \brief state 获取当前插件管理器状态
 * \return 返回状态枚举
 */
DFMExtPluginManager::State DFMExtPluginManager::state() const
{
    return d->currState;
}

/*!
 * \brief errorString 获取当前插件管理器出现的错误
 * \return 返回错误详细信息
 */
QString DFMExtPluginManager::errorString() const
{
    return d->errorString;
}

/*!
 * \brief instance 获取插件管理器实例
 * \return 静态区插件管理器实例
 */
DFMExtPluginManager &DFMExtPluginManager::instance()
{
    static DFMExtPluginManager manager;
    return manager;
}

/*!
 * \brief DFMExtPluginManager::extensionDefaultPath 获取扩展默认路径支持
 * \return
 */
QString DFMExtPluginManager::pluginDefaultPath()
{
    return d->pluginDefaultPath;
}

DFMExtPluginManager::DFMExtPluginManager(QObject *parent)
    : QObject (parent)
    , d(new DFMExtPluginManagerPrivate(this))
{

}

DFMExtPluginManagerPrivate::DFMExtPluginManagerPrivate(DFMExtPluginManager *qq)
    : QObject (qq)
    , q(qq)
    , pluginDefaultPath("")
    , pluginPaths({})
    , currState(DFMExtPluginManager::State::Invalid)
    , emblemIcons({})
    , menus({})
    , loaders({})
{
#ifdef EXTENSIONSDIR
        pluginDefaultPath = EXTENSIONSDIR;
#endif
}
