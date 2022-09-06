// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMEXTPLUGINMANAGER_P_H
#define DFMEXTPLUGINMANAGER_P_H

#include "dfmextpluginloader.h"
#include "dfmextpluginmanager.h"
#include "dfmextmenuimplproxy.h"
#include "interfaces/dfilesystemwatcher.h"

#include <QMap>
#include <QObject>
#include <QMutex>

#include <mutex>

class DFMExtPluginManager;
class DFMExtPluginManagerPrivate : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(DFMExtPluginManagerPrivate)
    friend class DFMExtPluginManager;
    DFMExtPluginManager *const q;

    QString pluginDefaultPath;
    DFMExtMenuImplProxy menuImplProxy;
    QStringList pluginPaths;
    DFMExtPluginManager::State currState;
    QString errorString;

    //! plugin interfaces cache
    DFMExtPluginManager::DFMExtEmblemIconMap emblemIcons;
    DFMExtPluginManager::DFMExtMenuMap menus;

    //! pluginName and loader cache
    QMap<QString, DFMExtPluginLoaderPointer> loaders;
    DFileSystemWatcher *extensionWathcer { nullptr };
    std::once_flag watcherFlag;
    QMutex mutex;

    QMap<QString, int> retryMap {};
    static const int kMaxRetryCount { 10 };
    static const int kDefaultWatiTime { 1000 };
public:
    explicit DFMExtPluginManagerPrivate(DFMExtPluginManager *qq);
    bool scanPlugin(const QString &path);
    void appendExtension(const QString &libName, const DFMExtPluginLoaderPointer &loader);
    void updateExtensionState(const QString &libName, DFMExtPluginManager::PluginLogicState state);

public slots:
    void onExtensionFileDeleted(const QString &path, const QString &name);
    void onExtensionFileCreated(const QString &path, const QString &name);
    void onExtensionFileCreatedLater(const QString &path, const QString &name, int ms);
    void onExtensionFileMoved(const QString &fromPath, const QString &fromName,
                              const QString &toPath, const QString &toName);
};

#endif   // DFMEXTPLUGINMANAGER_P_H
