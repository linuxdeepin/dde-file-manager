// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EXTENSIONPLUGINMANAGER_P_H
#define EXTENSIONPLUGINMANAGER_P_H

#include "extensionpluginmanager.h"

#include "extensionimpl/menuimpl/dfmextmenuimplproxy.h"

#include <dfm-base/base/schemefactory.h>

#include <QThread>
#include <QMap>

DPUTILS_BEGIN_NAMESPACE

class ExtensionPluginInitWorker : public QObject
{
    Q_OBJECT

public Q_SLOTS:
    void doWork(const QStringList &paths);

Q_SIGNALS:
    void requestInitPlugin(ExtPluginLoaderPointer);

    void scanPluginsFinished();
    void loadPluginsFinished();
    void initPluginsFinished();

private:
    std::map<QString, ExtPluginLoaderPointer> allLoaders;
    std::map<QString, ExtPluginLoaderPointer> loadedLoaders;
};

class ExtensionPluginManagerPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(ExtensionPluginManager)

public:
    using DFMExtMenuPluginMap = QMap<QString, DFMEXT::DFMExtMenuPlugin *>;
    using DFMExtEmblemPluginMap = QMap<QString, DFMEXT::DFMExtEmblemIconPlugin *>;
    using DFMExtWindowPluginMap = QMap<QString, DFMEXT::DFMExtWindowPlugin *>;
    using DFMExtFilePluginMap = QMap<QString, DFMEXT::DFMExtFilePlugin *>;

    explicit ExtensionPluginManagerPrivate(ExtensionPluginManager *qq);
    ~ExtensionPluginManagerPrivate() override;

    void startInitializePlugins();
    void startMonitorPlugins();
    void restartDesktop(const QUrl &url);
    void doAppendExt(const QString &name, ExtPluginLoaderPointer loader);
    void release();

Q_SIGNALS:
    void startInitialize(const QStringList &paths);

public:
    ExtensionPluginManager *q_ptr { nullptr };

    QThread workerThread;
    ExtensionPluginManager::InitState curState { ExtensionPluginManager::kReady };
    QString defaultPluginPath;
    DFMExtMenuPluginMap menuMap;
    DFMExtEmblemPluginMap emblemMap;
    DFMExtWindowPluginMap windowMap;
    DFMExtFilePluginMap fileMap;

    QScopedPointer<DFMEXT::DFMExtMenuProxy> proxy { new DFMExtMenuImplProxy };
    AbstractFileWatcherPointer extPluginsPathWatcher;
};

DPUTILS_END_NAMESPACE

#endif   // EXTENSIONPLUGINMANAGER_P_H
