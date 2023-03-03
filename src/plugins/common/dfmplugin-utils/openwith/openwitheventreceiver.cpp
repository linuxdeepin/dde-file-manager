// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "openwitheventreceiver.h"
#include "openwith/openwithdialog.h"

#include <dfm-framework/dpf.h>

using namespace dfmplugin_utils;

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
