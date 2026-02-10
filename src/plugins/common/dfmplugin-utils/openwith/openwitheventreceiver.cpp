// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "openwitheventreceiver.h"
#include "openwith/openwithdialog.h"

#include <dfm-framework/dpf.h>

#include <dfm-base/widgets/filemanagerwindowsmanager.h>

#include <QApplication>

using namespace dfmplugin_utils;

void OpenWithEventReceiver::initEventConnect()
{
    dpfSlotChannel->connect("dfmplugin_utils", "slot_OpenWith_ShowDialog", this, &OpenWithEventReceiver::showOpenWithDialog);
}

void OpenWithEventReceiver::showOpenWithDialog(quint64 winId, const QList<QUrl> &urls)
{
    QWidget *parentWidget { nullptr };
    if (winId != 0) {
        parentWidget  = FMWindowsIns.findWindowById(winId);
        if (!parentWidget) {
            QWidgetList topWidgets = qApp->topLevelWidgets();
            auto find = std::find_if(topWidgets.begin(), topWidgets.end(), [winId](QWidget *w){
                    return  w->internalWinId() == winId;
            });
            if (find != topWidgets.end()) {
                parentWidget = *find;
            }
        }
    }

    OpenWithDialog *d = new OpenWithDialog(urls, parentWidget);
    d->setAttribute(Qt::WA_DeleteOnClose);
    d->setDisplayPosition(OpenWithDialog::Center);
    d->exec();
}

OpenWithEventReceiver::OpenWithEventReceiver(QObject *parent)
    : QObject(parent)
{
}
