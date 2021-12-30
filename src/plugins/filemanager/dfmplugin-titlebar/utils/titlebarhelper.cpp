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
#include "titlebarhelper.h"
#include "events/titlebareventcaller.h"

#include "services/filemanager/titlebar/titlebar_defines.h"
#include "services/filemanager/windows/windowsservice.h"

#include <dfm-framework/framework.h>

DPTITLEBAR_USE_NAMESPACE
DSB_FM_USE_NAMESPACE

QMap<quint64, TitleBarWidget *> TitleBarHelper::kTitleBarMap {};

TitleBarWidget *TitleBarHelper::findTileBarByWindowId(quint64 windowId)
{
    if (!kTitleBarMap.contains(windowId))
        return nullptr;

    return kTitleBarMap[windowId];
}

void TitleBarHelper::addTileBar(quint64 windowId, TitleBarWidget *titleBar)
{
    QMutexLocker locker(&TitleBarHelper::mutex());
    if (!kTitleBarMap.contains(windowId))
        kTitleBarMap.insert(windowId, titleBar);
}

void TitleBarHelper::removeTitleBar(quint64 windowId)
{
    QMutexLocker locker(&TitleBarHelper::mutex());
    if (kTitleBarMap.contains(windowId))
        kTitleBarMap.remove(windowId);
}

quint64 TitleBarHelper::windowId(QWidget *sender)
{
    auto &ctx = dpfInstance.serviceContext();
    auto windowService = ctx.service<WindowsService>(WindowsService::name());
    return windowService->findWindowId(sender);
}

QMenu *TitleBarHelper::createSettingsMenu(quint64 id)
{
    QMenu *menu = new QMenu();

    QAction *action { new QAction(QObject::tr("New window")) };
    action->setData(TitleBar::MenuAction::kNewWindow);
    menu->addAction(action);

    menu->addSeparator();

    action = new QAction(QObject::tr("Connect to Server"));
    action->setData(TitleBar::MenuAction::kConnectToServer);
    menu->addAction(action);

    action = new QAction(QObject::tr("Set share password"));
    action->setData(TitleBar::MenuAction::kSetUserSharePassword);
    menu->addAction(action);

    action = new QAction(QObject::tr("Settings"));
    action->setData(TitleBar::MenuAction::kSettings);
    menu->addAction(action);

    QObject::connect(menu, &QMenu::triggered, [id](QAction *act) {
        TitleBarEventCaller::sendSettingsMenuTriggered(id, static_cast<TitleBar::MenuAction>(act->data().toInt()));
    });

    return menu;
}

QMutex &TitleBarHelper::mutex()
{
    static QMutex m;
    return m;
}
