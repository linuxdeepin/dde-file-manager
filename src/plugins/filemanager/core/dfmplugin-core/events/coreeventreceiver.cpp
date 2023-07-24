// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "coreeventreceiver.h"
#include "utils/corehelper.h"

#include <dfm-base/base/urlroute.h>

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
        qWarning() << "Invalid Url: " << url;
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
    qInfo("Start load plugins at runtime: ");
    std::for_each(names.begin(), names.end(), [](const QString &name) {
        Q_ASSERT(qApp->thread() == QThread::currentThread());
        qInfo() << "About to load plugin:" << name;
        auto plugin { DPF_NAMESPACE::LifeCycle::pluginMetaObj(name) };
        if (plugin)
            qInfo() << "Load result: " << DPF_NAMESPACE::LifeCycle::loadPlugin(plugin)
                    << "State: " << plugin->pluginState();
    });
    qInfo() << "End load plugins at runtime.";
}

void CoreEventReceiver::handleHeadless()
{
    Q_ASSERT(qApp->applicationName() == "dde-file-manager");

    CoreHelper::instance().cacheDefaultWindow();
}
