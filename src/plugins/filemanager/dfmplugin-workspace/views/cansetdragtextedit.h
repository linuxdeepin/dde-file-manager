/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#ifndef CANSETDRAGTEXTEDIT_H
#define CANSETDRAGTEXTEDIT_H

#include "dfmplugin_workspace_global.h"

#include <DTextEdit>

DWIDGET_USE_NAMESPACE
DPWORKSPACE_BEGIN_NAMESPACE

class CanSetDragTextEdit : public DTextEdit
{
    Q_OBJECT
    friend class ExpandedItem;

public:
    explicit CanSetDragTextEdit(QWidget *parent = nullptr);
    explicit CanSetDragTextEdit(const QString &text, QWidget *parent = nullptr);
    //set QTextEdit can drag
    void setDragEnabled(const bool &bdrag);
};

DPWORKSPACE_END_NAMESPACE

#endif   // CANSETDRAGTEXTEDIT_H
