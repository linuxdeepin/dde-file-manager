// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "extensionpluginmanager_p.h"

#include "config.h"   //cmake
#include "tools/upgrade/builtininterface.h"

#include <QDebug>
#include <QCoreApplication>
#include <QThread>
#include <QDir>
#include <QDirIterator>
#include <QDBusConnection>

#include <mutex>

DPUTILS_BEGIN_NAMESPACE
DFMBASE_USE_NAMESPACE

void ExtensionPluginInitWorker::doWork(const QStringList &paths)
{
    // do scan plugins
    fmInfo() << "Start scan extension lib paths: " << paths;
    std::for_each(paths.cbegin(), paths.cend(), [this](const QString &path) {
        QDirIterator itera(path, { "*.so" }, QDir::Files | QDir::NoSymLinks);
        if (!itera.hasNext())
            fmWarning() << "Cannot find extension lib at: " << path;
        while (itera.hasNext()) {
            itera.next();
            ExtPluginLoaderPointer ptr { new ExtensionPluginLoader(itera.filePath()) };
            allLoaders.insert({ itera.filePath(), ptr });
            fmInfo() << "Scaned extension plugin: " << itera.filePath();
        }
    });
    emit scanPluginsFinished();

    // do load plugins
    fmInfo() << "Start load extension plugins";
    for (const auto &[k, v] : allLoaders) {
        if (!v->loadPlugin()) {
            fmWarning() << "Load failed: " << v->fileName() << v->lastError();
            continue;
        }
        fmInfo() << "Loaded extension plugin:" << v->fileName();
        loadedLoaders.insert({ k, v });
    }
    emit loadPluginsFinished();

    // do init plugins
    fmInfo() << "Start init extension plugins";
    for (const auto &[k, v] : loadedLoaders)
        emit requestInitPlugin(v);

    emit initPluginsFinished();
}

ExtensionPluginManagerPrivate::ExtensionPluginManagerPrivate(ExtensionPluginManager *qq)
    : q_ptr(qq)
{
    // TODO(zhangs): more ext paths supported
#ifdef EXTENSIONS_PATH
    defaultPluginPath = EXTENSIONS_PATH;
#else
#    error You Should setting pluginDefaultPath
#endif
}

ExtensionPluginManagerPrivate::~ExtensionPluginManagerPrivate()
{
    release();
}

void ExtensionPluginManagerPrivate::startInitializePlugins()
{
    Q_Q(ExtensionPluginManager);
    qRegisterMetaType<ExtPluginLoaderPointer>("ExtPluginLoaderPointer");

    ExtensionPluginInitWorker *worker { new ExtensionPluginInitWorker };
    worker->moveToThread(&workerThread);
    connect(&workerThread, &QThread::finished, worker, &QObject::deleteLater);
    // run doInit in other thread
    connect(this, &ExtensionPluginManagerPrivate::startInitialize, worker, &ExtensionPluginInitWorker::doWork);
    connect(worker, &ExtensionPluginInitWorker::scanPluginsFinished, this, [this]() {
        curState = ExtensionPluginManager::kScanned;
    });
    connect(worker, &ExtensionPluginInitWorker::loadPluginsFinished, this, [this]() {
        curState = ExtensionPluginManager::kLoaded;
    });
    connect(worker, &ExtensionPluginInitWorker::initPluginsFinished, this, [this, q]() {
        curState = ExtensionPluginManager::kInitialized;
        emit q->allPluginsInitialized();
        release();
    });
    connect(worker, &ExtensionPluginInitWorker::requestInitPlugin, this, [this](ExtPluginLoaderPointer loader) {
        // Some plugins construct GUI object in `initialize`,
        // so must invoke `initialize` int the main thread here.
        Q_ASSERT(loader);
        Q_ASSERT(qApp->thread() == QThread::currentThread());
        if (!loader->initialize()) {
            fmWarning() << "init failed: " << loader->fileName() << loader->lastError();
            return;
        }
        fmInfo() << "Inited extension plugin:" << loader->fileName();
        doAppendExt(loader->fileName(), loader);
    });

    workerThread.start();
    emit startInitialize({ defaultPluginPath });
}

void ExtensionPluginManagerPrivate::startMonitorPlugins()
{
    // Watcher must init in main thread!
    Q_ASSERT(qApp->thread() == QThread::currentThread());
    if (qApp->applicationName() != "dde-desktop")
        return;
    extPluginsPathWatcher = WatcherFactory::create<AbstractFileWatcher>(
            QUrl::fromLocalFile(defaultPluginPath));
    if (!extPluginsPathWatcher)
        return;
    connect(extPluginsPathWatcher.data(), &AbstractFileWatcher::subfileCreated,
            this, [this](const QUrl &url) {
                fmWarning() << "Extension plugins path add: " << url;
                restartDesktop(url);
            });
    connect(extPluginsPathWatcher.data(), &AbstractFileWatcher::fileRename,
            this, [this](const QUrl &oldUrl, const QUrl &newUrl) {
                fmWarning() << "Extension plugins path rename: " << oldUrl << newUrl;
                restartDesktop(oldUrl);
            });
    connect(extPluginsPathWatcher.data(), &AbstractFileWatcher::fileDeleted,
            this, [this](const QUrl &url) {
                fmWarning() << "Extension plugins path remove: " << url;
                restartDesktop(url);
            });
    fmInfo() << "Monitor extension plugins path: " << defaultPluginPath;
    extPluginsPathWatcher->startWatcher();
}

void ExtensionPluginManagerPrivate::restartDesktop(const QUrl &url)
{
    using namespace dfm_upgrade;
    if (!url.toLocalFile().endsWith(".so"))
        return;

    QString libPath { QString(DFM_TOOLS_DIR) + "/libdfm-upgrade.so" };
    QLibrary lib(libPath);
    if (!lib.load()) {
        fmWarning() << "fail to load upgrade library:" << lib.errorString();
        return;
    }

    auto func = reinterpret_cast<UpgradeFunc>(lib.resolve("dfm_tools_upgrade_doRestart"));
    if (!func) {
        fmWarning() << "no upgrade function in :" << lib.fileName();
        return;
    }

    QMap<QString, QString> args;
    args.insert(dfm_upgrade::kArgDesktop, "dde-shell");

    int ret = func(args);

    if (ret < 0) {
        fmWarning() << "something error, exit current process.";
    } else {
        auto arguments = qApp->arguments();
        // remove first
        if (!arguments.isEmpty())
            arguments.pop_front();
        QDBusConnection::sessionBus().unregisterService("com.deepin.dde.desktop");
        fmInfo() << "restart self " << qApp->applicationFilePath() << arguments;
        QProcess::startDetached(qApp->applicationFilePath(), arguments);
        _Exit(-1);
    }
}

void ExtensionPluginManagerPrivate::doAppendExt(const QString &name, ExtPluginLoaderPointer loader)
{
    Q_ASSERT(loader);

    DFMEXT::DFMExtMenuPlugin *menu { loader->resolveMenuPlugin() };
    if (menu)
        menuMap.insert(name, menu);

    DFMEXT::DFMExtEmblemIconPlugin *emblem { loader->resolveEmblemPlugin() };
    if (emblem)
        emblemMap.insert(name, emblem);

    DFMEXT::DFMExtWindowPlugin *window { loader->resolveWindowPlugin() };
    if (window)
        windowMap.insert(name, window);

    DFMEXT::DFMExtFilePlugin *file { loader->resolveFilePlugin() };
    if (file)
        fileMap.insert(name, file);
}

void ExtensionPluginManagerPrivate::release()
{
    static std::once_flag flag;
    std::call_once(flag, [this]() {
        workerThread.quit();
        workerThread.wait();
    });
}

ExtensionPluginManager &ExtensionPluginManager::instance()
{
    static ExtensionPluginManager ins;
    return ins;
}

ExtensionPluginManager::InitState ExtensionPluginManager::currentState() const
{
    Q_D(const ExtensionPluginManager);

    return d->curState;
}

bool ExtensionPluginManager::initialized() const
{
    Q_D(const ExtensionPluginManager);

    return d->curState == kInitialized;
}

bool ExtensionPluginManager::exists(ExtensionPluginManager::ExtensionType type) const
{
    Q_D(const ExtensionPluginManager);

    switch (type) {
    case ExtensionType::kMenu:
        return !d->menuMap.isEmpty();
    case ExtensionType::kEmblemIcon:
        return !d->emblemMap.isEmpty();
    }

    return false;
}

QList<DFMEXT::DFMExtMenuPlugin *> ExtensionPluginManager::menuPlugins() const
{
    Q_D(const ExtensionPluginManager);

    return d->menuMap.values();
}

QList<DFMEXT::DFMExtEmblemIconPlugin *> ExtensionPluginManager::emblemPlugins() const
{
    Q_D(const ExtensionPluginManager);

    return d->emblemMap.values();
}

QList<DFMEXT::DFMExtWindowPlugin *> ExtensionPluginManager::windowPlugins() const
{
    Q_D(const ExtensionPluginManager);

    return d->windowMap.values();
}

QList<DFMEXT::DFMExtFilePlugin *> ExtensionPluginManager::filePlugins() const
{
    Q_D(const ExtensionPluginManager);

    return d->fileMap.values();
}

DFMEXT::DFMExtMenuProxy *ExtensionPluginManager::pluginMenuProxy() const
{
    Q_D(const ExtensionPluginManager);

    return d->proxy.data();
}

void ExtensionPluginManager::onLoadingPlugins()
{
    Q_ASSERT(qApp->thread() == QThread::currentThread());

    static std::once_flag flag;
    std::call_once(flag, [this]() {
        Q_D(ExtensionPluginManager);
        d->startInitializePlugins();
        // 插件监控功能提示框可能误导用户，暂时去除
#if 0
        d->startMonitorPlugins();
#endif
    });
}

ExtensionPluginManager::ExtensionPluginManager(QObject *parent)
    : QObject(parent), d_ptr(new ExtensionPluginManagerPrivate(this))
{
}

ExtensionPluginManager::~ExtensionPluginManager()
{
}

DPUTILS_END_NAMESPACE
