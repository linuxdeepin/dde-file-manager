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
    static TitleBarWidget *findTileBarByWindowId(quint64 windowId);
    static void addTileBar(quint64 windowId, TitleBarWidget *titleBar);
    static void removeTitleBar(quint64 windowId);
    static quint64 windowId(QWidget *sender);

    static QMenu *createSettingsMenu(quint64 id);
    static QList<CrumbData> crumbSeprateUrl(const QUrl &url);
    static QList<CrumbData> tansToCrumbDataList(const QList<QVariantMap> &mapGroup);
    static bool displayIcon();
    static void handlePressed(QWidget *sender, const QString &text, bool *isSearch = nullptr);

    static void showSettingsDialog(quint64 windowId);
    static void showConnectToServerDialog(quint64 windowId);
    static void showUserSharePasswordSettingDialog(quint64 windowId);

private:
    static QMutex &mutex();
    static void handleSettingMenuTriggered(quint64 windowId, int action);
    static QString getDisplayName(const QString &name);
    static QMap<quint64, TitleBarWidget *> kTitleBarMap;
};

}

#endif   // TITLEBARHELPER_H
