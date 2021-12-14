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
#ifndef SIDEBARSERVICE_P_H
#define SIDEBARSERVICE_P_H

#include "dfm_filemanager_service_global.h"

#include <QObject>
#include <QPointer>

DSB_FM_BEGIN_NAMESPACE

class SideBarService;
class SideBarServicePrivate : public QObject
{
    friend SideBarService;
    Q_OBJECT

public:
    explicit SideBarServicePrivate(SideBarService *serv);

private:
    QPointer<SideBarService> service;
};

DSB_FM_END_NAMESPACE

#endif   // SIDEBARSERVICE_P_H
