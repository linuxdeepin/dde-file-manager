// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EXTENSIONPLUGINMANAGER_P_H
#define EXTENSIONPLUGINMANAGER_P_H

#include "extensionpluginmanager.h"

#include "extensionimpl/menuimpl/dfmextmenuimplproxy.h"

#include <QThread>
#include <QMap>

DPUTILS_BEGIN_NAMESPACE

class ExtensionPluginInitWorker : public QObject
{
    Q_OBJECT

public Q_SLOTS:
    void doWork(const QStringList &paths);

private:
    void doAppendExt(const QString &name, ExtPluginLoaderPointer loader);

Q_SIGNALS:
    void scanPluginsFinished();
    void loadPluginsFinished();
    void initPluginsFinished();

    void newMenuPluginResolved(const QString &name, DFMEXT::DFMExtMenuPlugin *menu);
    void newEmblemPluginResolved(const QString &name, DFMEXT::DFMExtEmblemIconPlugin *emblem);

private:
    std::map<QString, ExtPluginLoaderPointer> allLoaders;
    std::map<QString, ExtPluginLoaderPointer> loadedLoaders;
};

class ExtensionPluginManagerPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(ExtensionPluginManager)

public:
    using DFMExtMenuPluginMap = QMap<QString, QSharedPointer<DFMEXT::DFMExtMenuPlugin>>;
    using DFMExtEmblemPluginMap = QMap<QString, QSharedPointer<DFMEXT::DFMExtEmblemIconPlugin>>;

    explicit ExtensionPluginManagerPrivate(ExtensionPluginManager *qq);
    ~ExtensionPluginManagerPrivate() override {}

    void startInitializePlugins();
    void startMonitorPlugins();

Q_SIGNALS:
    void startInitialize(const QStringList &paths);

public:
    ExtensionPluginManager *q_ptr { nullptr };

    QThread workerThread;
    ExtensionPluginManager::InitState curState { ExtensionPluginManager::kReady };
    QString defaultPluginPath;
    DFMExtMenuPluginMap menuMap;
    DFMExtEmblemPluginMap emblemMap;
    QScopedPointer<DFMEXT::DFMExtMenuProxy> proxy { new DFMExtMenuImplProxy };
};

DPUTILS_END_NAMESPACE

#endif   // EXTENSIONPLUGINMANAGER_P_H
