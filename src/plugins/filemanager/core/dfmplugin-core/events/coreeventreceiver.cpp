/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#include "coreeventreceiver.h"
#include "utils/corehelper.h"

#include "dfm-base/base/urlroute.h"

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
    CoreHelper::cd(windowId, url);
}

void CoreEventReceiver::handleOpenWindow(const QUrl &url)
{
    CoreHelper::openNewWindow(url);
}

void CoreEventReceiver::handleOpenWindow(const QUrl &url, const QVariant &opt)
{
    CoreHelper::openNewWindow(url, opt);
}

void CoreEventReceiver::handleLoadPlugins(const QStringList &names)
{
    std::for_each(names.begin(), names.end(), [](const QString &name) {
        Q_ASSERT(qApp->thread() == QThread::currentThread());
        qInfo() << "About to load plugin:" << name;
        auto plugin { DPF_NAMESPACE::LifeCycle::pluginMetaObj(name) };
        if (plugin)
            qInfo() << "Load result: " << DPF_NAMESPACE::LifeCycle::loadPlugin(plugin)
                    << "State: " << plugin->pluginState();
    });
}
