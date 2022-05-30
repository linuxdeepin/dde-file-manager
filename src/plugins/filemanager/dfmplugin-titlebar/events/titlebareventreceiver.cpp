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
#include "titlebareventreceiver.h"
#include "views/titlebarwidget.h"
#include "views/navwidget.h"

#include "utils/titlebarhelper.h"

DPTITLEBAR_USE_NAMESPACE
TitleBarEventReceiver *TitleBarEventReceiver::instance()
{
    static TitleBarEventReceiver receiver;
    return &receiver;
}

void TitleBarEventReceiver::handleTabAdded(quint64 windowId)
{
    TitleBarWidget *w = TitleBarHelper::findTileBarByWindowId(windowId);
    if (!w)
        return;
    w->navWidget()->addHistroyStack();
}

void TitleBarEventReceiver::handleTabChanged(quint64 windowId, int index)
{
    TitleBarWidget *w = TitleBarHelper::findTileBarByWindowId(windowId);
    if (!w)
        return;
    w->navWidget()->switchHistoryStack(index);
}

void TitleBarEventReceiver::handleTabMoved(quint64 windowId, int from, int to)
{
    TitleBarWidget *w = TitleBarHelper::findTileBarByWindowId(windowId);
    if (!w)
        return;
    w->navWidget()->moveNavStacks(from, to);
}

void TitleBarEventReceiver::handleTabRemovd(quint64 windowId, int index)
{
    TitleBarWidget *w = TitleBarHelper::findTileBarByWindowId(windowId);
    if (!w)
        return;
    w->navWidget()->removeNavStackAt(index);
}

void TitleBarEventReceiver::handleStartSpinner(quint64 windowId)
{
    TitleBarWidget *w = TitleBarHelper::findTileBarByWindowId(windowId);
    if (!w)
        return;
    w->startSpinner();
}

void TitleBarEventReceiver::handleStopSpinner(quint64 windowId)
{
    TitleBarWidget *w = TitleBarHelper::findTileBarByWindowId(windowId);
    if (!w)
        return;
    w->stopSpinner();
}

void TitleBarEventReceiver::handleShowFilterButton(quint64 windowId, bool visible)
{
    TitleBarWidget *w = TitleBarHelper::findTileBarByWindowId(windowId);
    if (!w)
        return;
    w->showSearchFilterButton(visible);
}

void TitleBarEventReceiver::handleViewModeChanged(quint64 windowId, int mode)
{
    TitleBarWidget *w = TitleBarHelper::findTileBarByWindowId(windowId);
    if (!w)
        return;

    w->setViewModeState(mode);
}

TitleBarEventReceiver::TitleBarEventReceiver(QObject *parent)
    : QObject(parent)
{
}
