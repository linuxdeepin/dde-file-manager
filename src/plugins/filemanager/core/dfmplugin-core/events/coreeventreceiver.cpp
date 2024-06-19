// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "coreeventreceiver.h"
#include "utils/corehelper.h"

#include <dfm-base/base/urlroute.h>
#include <dfm-base/utils/dialogmanager.h>

#include <dfm-framework/lifecycle/lifecycle.h>

#include <QDebug>
#include <QUrl>
#include <QCoreApplication>
#include <QThread>
#include <QTimer>

#include <functional>

DPCORE_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

CoreEventReceiver::CoreEventReceiver(QObject *parent)
    : QObject(parent)
{
}

CoreEventReceiver *CoreEventReceiver::instance()
{
    static CoreEventReceiver receiver;
    return &receiver;
}

void CoreEventReceiver::handleChangeUrl(quint64 windowId, const QUrl &url)
{
    if (!url.isValid()) {
        fmWarning() << "Invalid Url: " << url;
        return;
    }
    CoreHelper::instance().cd(windowId, url);
}

void CoreEventReceiver::handleOpenWindow(const QUrl &url)
{
    Q_ASSERT(qApp->applicationName() == "dde-file-manager");

    CoreHelper::instance().openWindow(url);
}

void CoreEventReceiver::handleOpenWindow(const QUrl &url, const QVariant &opt)
{
    Q_ASSERT(qApp->applicationName() == "dde-file-manager");

    CoreHelper::instance().openWindow(url, opt);
}

void CoreEventReceiver::handleLoadPlugins(const QStringList &names)
{
    fmInfo("Start load plugins at runtime: ");
    std::for_each(names.begin(), names.end(), [](const QString &name) {
        Q_ASSERT(qApp->thread() == QThread::currentThread());
        // TODO(zhangs): refactor search
        if (name == "dfmplugin-search") {
            QTimer::singleShot(2000, [name]() {
                CoreHelper::instance().loadPlugin(name);
            });
        } else {
            CoreHelper::instance().loadPlugin(name);
        }
    });
    fmInfo() << "End load plugins at runtime.";
}

void CoreEventReceiver::handleHeadless()
{
    Q_ASSERT(qApp->applicationName() == "dde-file-manager");

    // The purpose of hot-launch is to start the process faster,
    // if you just cache the plugin loading, the speed is not ideal.
    // So here you need to cache an assembled window.
    CoreHelper::instance().cacheDefaultWindow();
}

void CoreEventReceiver::handleShowSettingDialog(quint64 windowId)
{
    auto handle = FMQuickWindowIns->findWindowById(windowId);

    if (!handle) {
        fmWarning() << "Invalid window id: " << windowId;
        return;
    }

    // TODO: adapt qml
    // DialogManagerInstance->showSetingsDialog(handle->window());
}
