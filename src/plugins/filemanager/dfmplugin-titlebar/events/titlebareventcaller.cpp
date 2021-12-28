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
#include "titlebareventcaller.h"
#include "utils/titlebarhelper.h"

#include <dfm-framework/framework.h>

DPTITLEBAR_USE_NAMESPACE
DSB_FM_USE_NAMESPACE

void TitleBarEventCaller::sendViewMode(QWidget *sender, TitleBar::ViewMode mode)
{
    quint64 id = TitleBarHelper::windowId(sender);
    Q_ASSERT(id > 0);
    dpf::Event event;
    event.setTopic(TitleBar::EventTopic::kTitleBarOptionBtn);
    event.setData(TitleBar::EventData::kSwitchMode);
    event.setProperty(TitleBar::EventProperty::kWindowId, id);
    event.setProperty(TitleBar::EventProperty::kViewMode, mode);
    qDebug() << "send event: " << event;
    dpfInstance.eventProxy().pubEvent(event);
}
