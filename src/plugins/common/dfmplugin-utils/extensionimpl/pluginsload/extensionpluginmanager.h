// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EXTENSIONPLUGINMANAGER_H
#define EXTENSIONPLUGINMANAGER_H

#include "dfmplugin_utils_global.h"

#include "extensionpluginloader.h"

#include <QObject>

DPUTILS_BEGIN_NAMESPACE

class DFMExtMenuImplProxy;
class ExtensionPluginManagerPrivate;
class ExtensionPluginManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ExtensionPluginManager)
    Q_DECLARE_PRIVATE(ExtensionPluginManager)

public:
    enum InitState {
        kReady,
        kScanned,
        kLoaded,
        kInitialized
    };

    enum ExtensionType {
        kMenu,
        kEmblemIcon
    };

    static ExtensionPluginManager &instance();
    InitState currentState() const;
    bool initialized() const;
    bool exists(ExtensionType type) const;
    QList<DFMEXT::DFMExtMenuPlugin *> menuPlugins() const;
    QList<DFMEXT::DFMExtEmblemIconPlugin *> emblemPlugins() const;
    QList<DFMEXT::DFMExtWindowPlugin *> windowPlugins() const;
    QList<DFMEXT::DFMExtFilePlugin *> filePlugins() const;
    DFMEXT::DFMExtMenuProxy *pluginMenuProxy() const;

Q_SIGNALS:
    void requestInitlaizePlugins();
    void allPluginsInitialized();

public Q_SLOTS:
    void onLoadingPlugins();

private:
    explicit ExtensionPluginManager(QObject *parent = nullptr);
    ~ExtensionPluginManager() override;

private:
    QScopedPointer<ExtensionPluginManagerPrivate> d_ptr;
};

DPUTILS_END_NAMESPACE

#endif   // EXTENSIONPLUGINMANAGER_H
