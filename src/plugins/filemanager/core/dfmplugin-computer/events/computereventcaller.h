// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMPUTEREVENTCALLER_H
#define COMPUTEREVENTCALLER_H

#include "dfmplugin_computer_global.h"
#include <dfm-base/file/entry/entryfileinfo.h>

#include <QUrl>

class QWidget;
namespace dfmplugin_computer {

class ComputerEventCaller
{
public:
    ComputerEventCaller() = delete;

    static void cdTo(QWidget *sender, const QUrl &url);
    static void cdTo(QWidget *sender, const QString &path);
    static void cdTo(quint64 winId, const QUrl &url);
    static void cdTo(quint64 winId, const QString &path);

    static void sendEnterInNewWindow(const QUrl &url);
    static void sendEnterInNewTab(quint64 winId, const QUrl &url);

    static void sendContextActionTriggered(quint64 winId, const QUrl &url, const QString &action);
    static void sendOpenItem(quint64 winId, const QUrl &url);
    static void sendCtrlNOnItem(quint64 winId, const QUrl &url);
    static void sendCtrlTOnItem(quint64 winId, const QUrl &url);
    static void sendShowPropertyDialog(const QList<QUrl> &urls);
    static void sendErase(const QString &dev);
};

}

#endif   // COMPUTEREVENTCALLER_H
