// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "extensionpluginmanager_p.h"

#include <QDebug>
#include <QCoreApplication>
#include <QThread>
#include <QDir>
#include <QDirIterator>

#include <mutex>

DPUTILS_BEGIN_NAMESPACE

void ExtensionPluginInitWorker::doWork(const QStringList &paths)
{
    // do scan plugins
    qInfo() << "Start scan extension lib paths: " << paths;
    std::for_each(paths.cbegin(), paths.cend(), [this](const QString &path) {
        QDirIterator itera(path, { "*.so" }, QDir::Files | QDir::NoSymLinks);
        if (!itera.hasNext())
            qWarning() << "Cannot find extension lib at: " << path;
        while (itera.hasNext()) {
            itera.next();
            ExtPluginLoaderPointer ptr { new ExtensionPluginLoader(itera.filePath()) };
            allLoaders.insert({ itera.filePath(), ptr });
            qInfo() << "Scaned extension plugin: " << itera.filePath();
        }
    });
    emit scanPluginsFinished();

    // do load plugins
    qInfo() << "Start load extension plugins";
    for (const auto &[k, v] : allLoaders) {
        if (!v->loadPlugin()) {
            qWarning() << "Load failed: " << v->fileName() << v->lastError();
            continue;
        }
        qInfo() << "Loaded extension plugin:" << v->fileName();
        loadedLoaders.insert({ k, v });
    }
    emit loadPluginsFinished();

    // do init plugins
    qInfo() << "Start init extension plugins";
    for (const auto &[k, v] : loadedLoaders) {
        if (!v->initialize()) {
            qWarning() << "init failed: " << v->fileName() << v->lastError();
            continue;
        }
        qInfo() << "Inited extension plugin:" << v->fileName();
        doAppendExt(v->fileName(), v);
    }
    // TODO(zhangs): record plugin state

    emit initPluginsFinished();
}

void ExtensionPluginInitWorker::doAppendExt(const QString &name, ExtPluginLoaderPointer loader)
{
    Q_ASSERT(loader);

    DFMEXT::DFMExtMenuPlugin *menu { loader->resolveMenuPlugin() };
    if (menu)
        emit newMenuPluginResolved(name, menu);

    DFMEXT::DFMExtEmblemIconPlugin *emblem { loader->resolveEmblemPlugin() };
    if (emblem)
        emit newEmblemPluginResolved(name, emblem);
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

void ExtensionPluginManagerPrivate::startInitializePlugins()
{
    Q_Q(ExtensionPluginManager);

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
        workerThread.quit();
        workerThread.wait();
    });
    connect(worker, &ExtensionPluginInitWorker::newMenuPluginResolved, this, [this](const QString &name, DFMEXT::DFMExtMenuPlugin *menu) {
        menuMap.insert(name, QSharedPointer<DFMEXT::DFMExtMenuPlugin>(menu));
    });
    connect(worker, &ExtensionPluginInitWorker::newEmblemPluginResolved, this, [this](const QString &name, DFMEXT::DFMExtEmblemIconPlugin *emblem) {
        emblemMap.insert(name, QSharedPointer<DFMEXT::DFMExtEmblemIconPlugin>(emblem));
    });

    workerThread.start();
    emit startInitialize({ defaultPluginPath });
}

void ExtensionPluginManagerPrivate::startMonitorPlugins()
{
    // TODO(zhangs): load plugin when running
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

QList<QSharedPointer<dfmext::DFMExtMenuPlugin>> ExtensionPluginManager::menuPlugins() const
{
    Q_D(const ExtensionPluginManager);

    return d->menuMap.values();
}

QList<QSharedPointer<dfmext::DFMExtEmblemIconPlugin>> ExtensionPluginManager::emblemPlugins() const
{
    Q_D(const ExtensionPluginManager);

    return d->emblemMap.values();
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
        d->startMonitorPlugins();
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
