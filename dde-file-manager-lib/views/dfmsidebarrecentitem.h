/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     Rekols <rekols@foxmail.com>
 *
 * Maintainer: Rekols <rekols@foxmail.com>
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

#ifndef DFMSIDEBARRECENTITEM_H
#define DFMSIDEBARRECENTITEM_H

#include "dfmglobal.h"
#include "dfmsidebardefaultitem.h"

DFM_BEGIN_NAMESPACE

class DFMSideBarRecentItem : public DFMSideBarDefaultItem
{
    Q_OBJECT
public:
    DFMSideBarRecentItem(DFMStandardPaths::StandardLocation location, QWidget *parent = nullptr);

protected:
    QMenu *createStandardContextMenu() const override;
};

DFM_END_NAMESPACE

#endif // DFMSIDEBARRECENTITEM_H
