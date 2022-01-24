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
#include "durl.h"
#include "dfmglobal.h"

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
 * \brief scanPlugins 扫描已设置路径下的所有插件
 * \return 返回执行结果，如果扫描路径下存在不可失败的so文件将返回false
 */
bool DFMExtPluginManager::scanPlugins()
{
    for (auto val : d->pluginPaths) {
        QDirIterator itera(val, QDir::Files | QDir::NoSymLinks);
        while (itera.hasNext()) {
            itera.next();
            d->scanPlugin(itera.filePath());
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
        if (!val->loadPlugin()) {
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
        if (!val->initialize()) {
            ret = false;
            qInfo() << val->errorString();
        }
        d->appendExtension(val->fileName(), val);
    }
    d->currState = DFMExtPluginManager::State::Initialized;
    return ret;
}

bool DFMExtPluginManager::monitorPlugins()
{
    std::call_once(d->watcherFlag, [this]() {
        // Watcher must init in main thread!
        DThreadUtil::runInMainThread([this] {
            d->extensionWathcer = new DFileSystemWatcher(this);
            if (!d->extensionWathcer)
                return false;

            connect(d->extensionWathcer, &DFileSystemWatcher::fileDeleted, d, &DFMExtPluginManagerPrivate::onExtensionFileDeleted, Qt::DirectConnection);
            connect(d->extensionWathcer, &DFileSystemWatcher::fileCreated, d,
                    [this](const QString &path, const QString &name) {
                        d->onExtensionFileCreatedLater(path, name, DFMExtPluginManagerPrivate::kDefaultWatiTime);
                    },
                    Qt::DirectConnection);
            connect(d->extensionWathcer, &DFileSystemWatcher::fileMoved, d, &DFMExtPluginManagerPrivate::onExtensionFileMoved, Qt::DirectConnection);
            return d->extensionWathcer->addPath(d->pluginDefaultPath);
        });
    });
    return true;
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
DFMExtPluginManager::DFMExtMenus DFMExtPluginManager::menus() const
{
    QMutexLocker guard(&d->mutex);
    DFMExtMenus menuList;

    for (DFMExtMenuState menu : d->menus.values()) {
        if (menu.first == DFMExtPluginManager::Enable)
            menuList.append(menu.second);
    }

    return menuList;
}

/*!
 * \brief emblemIcons 获取插件内部角标扩展接口
 * \return 返回接口列表
 */
DFMExtPluginManager::DFMExtEmblemIcons DFMExtPluginManager::emblemIcons() const
{
    QMutexLocker guard(&d->mutex);
    DFMExtEmblemIcons iconList;

    for (DFMExtEmblemState icon : d->emblemIcons.values()) {
        if (icon.first == DFMExtPluginManager::Enable)
            iconList.append(icon.second);
    }

    return iconList;
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
    : QObject(parent), d(new DFMExtPluginManagerPrivate(this))
{
}

DFMExtPluginManagerPrivate::DFMExtPluginManagerPrivate(DFMExtPluginManager *qq)
    : QObject(qq), q(qq), pluginDefaultPath(""), pluginPaths({}), currState(DFMExtPluginManager::State::Invalid), emblemIcons({}), menus({}), loaders({})
{
#ifdef EXTENSIONSDIR
    pluginDefaultPath = EXTENSIONSDIR;
#else
#    error You Should setting pluginDefaultPath
#endif
    pluginPaths.append(pluginDefaultPath);
}

bool DFMExtPluginManagerPrivate::scanPlugin(const QString &path)
{
    if (DFMExtPluginLoader::hasSymbol(path)) {
        DFMExtPluginLoaderPointer autoPointer(new DFMExtPluginLoader(path));
        loaders.insert(path, autoPointer);
        return true;
    }
    return false;
}

void DFMExtPluginManagerPrivate::appendExtension(const QString &libName, const DFMExtPluginLoaderPointer &loader)
{
    Q_ASSERT(loader);
    QMutexLocker guard(&mutex);
    auto menu = loader->extMenuPlugin();
    if (!menu.isNull()) {
        DFMExtPluginManager::DFMExtMenuState state(DFMExtPluginManager::Enable, menu);
        menus.insert(libName, state);
    }

    auto embleIcon = loader->extEmbleIconPlugin();
    if (!embleIcon.isNull()) {
        DFMExtPluginManager::DFMExtEmblemState state(DFMExtPluginManager::Enable, embleIcon);
        emblemIcons.insert(libName, state);
    }
}

void DFMExtPluginManagerPrivate::updateExtensionState(const QString &libName, DFMExtPluginManager::PluginLogicState state)
{
    QMutexLocker guard(&mutex);
    if (menus.contains(libName))
        menus[libName].first = state;

    if (emblemIcons.contains(libName))
        emblemIcons[libName].first = state;
}

void DFMExtPluginManagerPrivate::onExtensionFileDeleted(const QString &path, const QString &name)
{
    qInfo() << "Remove plugin file: " << path << name;
    if (path != pluginDefaultPath)
        return;
    QString libName { path + "/" + name };
    if (loaders.contains(libName)) {   // 只需要 disbale 插件的能力
        qInfo() << "Disbale pugin: " << libName;
        updateExtensionState(libName, DFMExtPluginManager::Disbale);
        Q_EMIT q->extensionPluginDisbale(libName);
    }
}

void DFMExtPluginManagerPrivate::onExtensionFileCreated(const QString &path, const QString &name)
{
    qInfo() << "Add plugin file: " << path << name;
    if (path != pluginDefaultPath)
        return;
    QString libName { path + "/" + name };
    if (loaders.contains(libName)) {   // 插件曾经被加载，然后被删除了，此时只是功能上被屏蔽了，因此只需要 enebale 功能
        qInfo() << "Enable pugin: " << libName;
        updateExtensionState(libName, DFMExtPluginManager::Enable);
        Q_EMIT q->extensionPluginEnable(libName);
    } else {   // 插件第一次安装，因此需要加载并初始化它
        if (!scanPlugin(libName)) {
            // 由于此时无法确保插件能够拷贝完成，因此尝试重复读取插件
            if (!retryMap.contains(libName))
                retryMap.insert(libName, 0);
            if (retryMap.value(libName) < kMaxRetryCount) {
                qInfo() << "Retry read plugin: " << libName << retryMap[libName];
                onExtensionFileCreatedLater(path, name, kDefaultWatiTime);
                retryMap[libName]++;
                return;
            }
            qWarning() << "The plugin " << libName << " has not symbol!";
            return;
        }
        DFMExtPluginLoaderPointer loader { loaders[libName] };
        Q_ASSERT(loader);
        if (!loader->loadPlugin()) {
            loaders.remove(libName);
            qWarning() << "The plugin" << libName << "load failed: " << loader->errorString();
            return;
        }
        if (!loader->initialize()) {
            loaders.remove(libName);
            qWarning() << "The plugin" << libName << "init failed: " << loader->errorString();
            return;
        }
        appendExtension(libName, loader);
        Q_EMIT q->extensionPluginCreated(libName);
    }
}

void DFMExtPluginManagerPrivate::onExtensionFileCreatedLater(const QString &path, const QString &name, int ms)
{
    // created 信号并不代表扩展库拷贝完成，加载将会失败。这里延迟一定时间，绝大部分扩展库能够拷贝完成
    QTimer::singleShot(ms, this, [this, path, name]() {
        this->onExtensionFileCreated(path, name);
    });
}

void DFMExtPluginManagerPrivate::onExtensionFileMoved(const QString &fromPath, const QString &fromName, const QString &toPath, const QString &toName)
{
    if (fromPath == pluginDefaultPath) {
        onExtensionFileDeleted(fromPath, fromName);
        return;
    }

    if (toPath == pluginDefaultPath) {
        onExtensionFileCreatedLater(toPath, toName, DFMExtPluginManagerPrivate::kDefaultWatiTime);
        return;
    }
}
