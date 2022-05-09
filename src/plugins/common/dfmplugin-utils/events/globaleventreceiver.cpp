/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
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
#include "globaleventreceiver.h"

#include "dfm-base/dfm_event_defines.h"

#include <dfm-framework/framework.h>

#include <QUrl>
#include <QDir>
#include <QProcess>

DPUTILS_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

GlobalEventReceiver::GlobalEventReceiver(QObject *parent)
    : QObject(parent)
{
}

GlobalEventReceiver *GlobalEventReceiver::instance()
{
    static GlobalEventReceiver receiver;
    return &receiver;
}

void GlobalEventReceiver::initEventConnect()
{
    dpfInstance.eventDispatcher().subscribe(GlobalEventType::kOpenAsAdmin,
                                            GlobalEventReceiver::instance(), &GlobalEventReceiver::handleOpenAsAdmin);
}

void GlobalEventReceiver::handleOpenAsAdmin(const QUrl &url)
{
    if (url.isEmpty() || !url.isValid()) {
        qWarning() << "Invalid Url: " << url;
        return;
    }

    QString localPath { url.toLocalFile() };
    if (!QDir(localPath).exists()) {
        qWarning() << "Url path not exists: " << localPath;
        return;
    }

    QProcess::startDetached("dde-file-manager-pkexec", { localPath });
}
