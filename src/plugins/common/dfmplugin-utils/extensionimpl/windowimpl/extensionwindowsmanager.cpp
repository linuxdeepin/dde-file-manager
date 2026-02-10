// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "extensionwindowsmanager.h"
#include "extensionimpl/pluginsload/extensionpluginmanager.h"

#include <dfm-base/widgets/filemanagerwindowsmanager.h>

#include <QUrl>
#include <QTimer>

#include <mutex>

DPUTILS_BEGIN_NAMESPACE
DFMBASE_USE_NAMESPACE

static void doActionForEveryPlugin(std::function<void(DFMEXT::DFMExtWindowPlugin *)> callback)
{
    Q_ASSERT(callback);
    if (!ExtensionPluginManager::instance().initialized()) {
        fmWarning() << "The event occurs before any plugin initialization is complete";
        return;
    }
    const auto &windowPlugins { ExtensionPluginManager::instance().windowPlugins() };
    std::for_each(windowPlugins.begin(), windowPlugins.end(), [callback](DFMEXT::DFMExtWindowPlugin *plugin) {
        Q_ASSERT(plugin);
        callback(plugin);
    });
}

ExtensionWindowsManager &ExtensionWindowsManager::instance()
{
    static ExtensionWindowsManager ins;
    return ins;
}

void ExtensionWindowsManager::initialize()
{
    connect(&FMWindowsIns, &FileManagerWindowsManager::windowOpened,
            this, &ExtensionWindowsManager::onWindowOpened);
    connect(&FMWindowsIns, &FileManagerWindowsManager::windowClosed,
            this, &ExtensionWindowsManager::onWindowClosed);
    connect(&FMWindowsIns, &FileManagerWindowsManager::lastWindowClosed,
            this, &ExtensionWindowsManager::onLastWindowClosed);
    connect(&FMWindowsIns, &FileManagerWindowsManager::currentUrlChanged,
            this, &ExtensionWindowsManager::onCurrentUrlChanged);
    connect(&ExtensionPluginManager::instance(), &ExtensionPluginManager::allPluginsInitialized,
            this, &ExtensionWindowsManager::onAllPluginsInitialized);
}

void ExtensionWindowsManager::onWindowOpened(quint64 id)
{
    if (ExtensionPluginManager::instance().initialized()) {
        handleWindowOpened(id);
    } else {
        firstWinId = id;
        QTimer::singleShot(200, this, []() {
            emit ExtensionPluginManager::instance().requestInitlaizePlugins();
        });
    }
}

void ExtensionWindowsManager::onWindowClosed(quint64 id)
{
    doActionForEveryPlugin([id](DFMEXT::DFMExtWindowPlugin *plugin) {
        plugin->windowClosed(id);
    });
}

void ExtensionWindowsManager::onLastWindowClosed(quint64 id)
{
    doActionForEveryPlugin([id](DFMEXT::DFMExtWindowPlugin *plugin) {
        plugin->lastWindowClosed(id);
    });
}

void ExtensionWindowsManager::onCurrentUrlChanged(quint64 id, const QUrl &url)
{
    if (!ExtensionPluginManager::instance().initialized())
        return;
    std::string urlStr { url.toString().toStdString() };
    doActionForEveryPlugin([id, urlStr](DFMEXT::DFMExtWindowPlugin *plugin) {
        plugin->windowUrlChanged(id, urlStr);
    });
}

void ExtensionWindowsManager::onAllPluginsInitialized()
{
    if (firstWinId != 0)
        handleWindowOpened(firstWinId);
    // TODO(zhangs): add proxy
}

ExtensionWindowsManager::ExtensionWindowsManager(QObject *parent)
    : QObject(parent)
{
}

void ExtensionWindowsManager::handleWindowOpened(quint64 id)
{
    Q_ASSERT(id != 0);
    static std::once_flag flag;
    std::call_once(flag, [id]() {
        doActionForEveryPlugin([id](DFMEXT::DFMExtWindowPlugin *plugin) {
            plugin->firstWindowOpened(id);
        });
    });

    doActionForEveryPlugin([id](DFMEXT::DFMExtWindowPlugin *plugin) {
        plugin->windowOpened(id);
    });
}

DPUTILS_END_NAMESPACE
