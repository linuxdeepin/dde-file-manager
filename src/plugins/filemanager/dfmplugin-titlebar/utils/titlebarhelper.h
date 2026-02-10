// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TITLEBARHELPER_H
#define TITLEBARHELPER_H

#include "dfmplugin_titlebar_global.h"

#include <QMap>
#include <QMutex>
#include <QWidget>
#include <QMenu>

namespace dfmplugin_titlebar {

class TitleBarWidget;
class TitleBarHelper
{
public:
    static QList<TitleBarWidget *> titlebars();
    static TitleBarWidget *findTileBarByWindowId(quint64 windowId);
    static void addTileBar(quint64 windowId, TitleBarWidget *titleBar);
    static void removeTitleBar(quint64 windowId);
    static quint64 windowId(QWidget *sender);

    static void createSettingsMenu(quint64 id);
    static QList<CrumbData> crumbSeprateUrl(const QUrl &url);
    static QList<CrumbData> tansToCrumbDataList(const QList<QVariantMap> &mapGroup);
    static void handleJumpToPressed(QWidget *sender, const QString &text);
    static void handleSearch(QWidget *sender, const QString &text);

    static void openCurrentUrlInNewTab(quint64 windowId);
    static void showSettingsDialog(quint64 windowId);
    static void showConnectToServerDialog(quint64 windowId);
    static void showUserSharePasswordSettingDialog(quint64 windowId);
    static void showDiskPasswordChangingDialog(quint64 windowId);

    static void registerKeepTitleStatusScheme(const QString &scheme);
    static bool checkKeepTitleStatus(const QUrl &url);

    static void registerViewModelUrlCallback(const QString &scheme, ViewModeUrlCallback callback);
    static ViewModeUrlCallback viewModelUrlCallback(const QUrl &url);

    // FileViewState unified access methods
    static QVariant getFileViewStateValue(const QUrl &url, const QString &key, const QVariant &defaultValue = QVariant());
    static void setFileViewStateValue(const QUrl &url, const QString &key, const QVariant &value);
    static QUrl transformViewModeUrl(const QUrl &url);

    // View mode switching helper methods
    static bool isTreeViewGloballyEnabled();
    static bool isViewModeVisibleForScheme(int mode, const QString &scheme);

public:
    static bool newWindowAndTabEnabled;
    static bool searchEnabled;

private:
    static QMutex &mutex();
    static void handleSettingMenuTriggered(quint64 windowId, int action);
    static QString getDisplayName(const QString &name);
    static QMap<quint64, TitleBarWidget *> kTitleBarMap;
    static QList<QString> kKeepTitleStatusSchemeList;
    static QMap<QString, ViewModeUrlCallback> kViewModeUrlCallbackMap;
};

}

#endif   // TITLEBARHELPER_H
