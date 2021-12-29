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

DPTITLEBAR_BEGIN_NAMESPACE

class TitleBarWidget;
class TitleBarHelper
{
public:
    static TitleBarWidget *findTileBarByWindowId(quint64 windowId);
    static void addTileBar(quint64 windowId, TitleBarWidget *titleBar);
    static void removeTileBar(quint64 windowId);
    static quint64 windowId(QWidget *sender);
    static QMenu *createSettingsMenu(quint64 id);

private:
    static QMutex &mutex();
    static QMap<quint64, TitleBarWidget *> kTitleBarMap;
};

DPTITLEBAR_END_NAMESPACE

#endif   // TITLEBARHELPER_H
