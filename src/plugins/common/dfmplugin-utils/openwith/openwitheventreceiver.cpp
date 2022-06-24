/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: lixiang<lixianga@uniontech.com>
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
#include "openwitheventreceiver.h"
#include "openwith/openwithdialog.h"

#include <dfm-framework/dpf.h>

DPUTILS_USE_NAMESPACE

void OpenWithEventReceiver::initEventConnect()
{
    dpfSlotChannel->connect("dfmplugin_utils", "slot_OpenWith_ShowDialog", this, &OpenWithEventReceiver::showOpenWithDialog);
}

void OpenWithEventReceiver::showOpenWithDialog(const QList<QUrl> &urls)
{
    OpenWithDialog *d = new OpenWithDialog(urls);
    d->setDisplayPosition(OpenWithDialog::Center);
    d->open();
}

OpenWithEventReceiver::OpenWithEventReceiver(QObject *parent)
    : QObject(parent)
{
}
