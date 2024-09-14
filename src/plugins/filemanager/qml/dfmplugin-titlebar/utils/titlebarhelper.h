// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TITLEBARHELPER_H
#define TITLEBARHELPER_H

#include "dfmplugin_titlebar_global.h"

#include <dfm-gui/applet.h>

#include <QMap>
#include <QMutex>
#include <QWidget>
#include <QMenu>

namespace dfmplugin_titlebar {

class TitleBarWidget;
class TitleBarHelper
{
public:
    static TitleBarWidget *findTileBarByWindowId(quint64 windowId);
    static void addTileBar(quint64 windowId, TitleBarWidget *titleBar);
    static void removeTitleBar(quint64 windowId);
    static quint64 windowId(QWidget *sender);

    static quint64 windowId(dfmgui::Applet *applet);

    static void createSettingsMenu(quint64 id);
    static QList<CrumbData> crumbSeprateUrl(const QUrl &url);
    static QList<CrumbData> tansToCrumbDataList(const QList<QVariantMap> &mapGroup);
    static bool displayIcon();
    static void handlePressed(QWidget *sender, const QString &text, bool *isSearch = nullptr);

    static void openCurrentUrlInNewTab(quint64 windowId);
    static void showSettingsDialog(quint64 windowId);
    static void showConnectToServerDialog(quint64 windowId);
    static void showUserSharePasswordSettingDialog(quint64 windowId);
    static void showDiskPasswordChangingDialog(quint64 windowId);

public:
    static bool newWindowAndTabEnabled;

private:
    static QMutex &mutex();
    static void handleSettingMenuTriggered(quint64 windowId, int action);
    static QString getDisplayName(const QString &name);
    static QMap<quint64, TitleBarWidget *> kTitleBarMap;
};

}

#endif   // TITLEBARHELPER_H
