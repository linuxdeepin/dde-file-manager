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
#ifndef SIDEBAREVENTCALLER_H
#define SIDEBAREVENTCALLER_H

#include "dfmplugin_sidebar_global.h"

#include <QObject>

class QUrl;
class QWidget;
DPSIDEBAR_BEGIN_NAMESPACE

class SideBarEventCaller
{
    SideBarEventCaller() = delete;

public:
    static void sendItemActived(QWidget *sender, const QUrl &url);
};

DPSIDEBAR_END_NAMESPACE

#endif   // SIDEBAREVENTCALLER_H
