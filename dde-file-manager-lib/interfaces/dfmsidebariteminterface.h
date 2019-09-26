/*
 * Copyright (C) 2019 Deepin Technology Co., Ltd.
 *
 * Author:     Gary Wang <wzc782970009@gmail.com>
 *
 * Maintainer: Gary Wang <wangzichong@deepin.com>
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

#pragma once

#include <QMenu>
#include <QObject>
#include "dfmglobal.h"

DFM_BEGIN_NAMESPACE

class DFMSideBar;
class DFMSideBarItem;
class DFMSideBarItemInterface : public QObject
{
    Q_GADGET
public:
    explicit DFMSideBarItemInterface(QObject *parent = nullptr);

    virtual void cdAction(const DFMSideBar *sidebar, const DFMSideBarItem* item);
    virtual QMenu * contextMenu(const DFMSideBar *sidebar, const DFMSideBarItem* item);
    virtual void rename(const DFMSideBarItem *item, QString name);
};

DFM_END_NAMESPACE
