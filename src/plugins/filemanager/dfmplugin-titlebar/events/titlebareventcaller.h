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
#ifndef TITLEBAREVENTCALLER_H
#define TITLEBAREVENTCALLER_H

#include "dfmplugin_titlebar_global.h"

#include "services/filemanager/titlebar/titlebar_defines.h"
#include "dfm-base/dfm_global_defines.h"

#include <QObject>

DPTITLEBAR_BEGIN_NAMESPACE

class TitleBarEventCaller
{
    TitleBarEventCaller() = delete;

public:
    static void sendViewMode(QWidget *sender, DFMBASE_NAMESPACE::Global::ViewMode mode);
    static void sendSettingsMenuTriggered(quint64 windowId, DSB_FM_NAMESPACE::TitleBar::MenuAction action);
    static void sendDetailViewState(QWidget *sender, bool checked);
    static void sendCd(QWidget *sender, const QUrl &url);
    static void sendOpenWindow(const QUrl &url);
    static void sendOpenTab(quint64 windowId, const QUrl &url);
    static void sendSearch(QWidget *sender, const QString &keyword);
};

DPTITLEBAR_END_NAMESPACE

#endif   // TITLEBAREVENTCALLER_H
