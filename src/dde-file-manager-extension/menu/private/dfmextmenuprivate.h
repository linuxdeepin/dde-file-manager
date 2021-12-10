/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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

#ifndef DFMEXTMENUPRIVATE_H
#define DFMEXTMENUPRIVATE_H

#include "dfm-extension-global.h"
#include "menu/dfmextmenu.h"

#include <string>
#include <list>

BEGEN_DFMEXT_NAMESPACE

class DFMExtAction;
class DFMExtMenuPrivate
{
    friend class DFMExtMenu;
public:
    explicit DFMExtMenuPrivate();
    virtual ~DFMExtMenuPrivate();

    virtual std::string title() const = 0;
    virtual void setTitle(const std::string &title) = 0;

    virtual std::string icon() const = 0;
    virtual void setIcon(const std::string &iconName) = 0;

    virtual bool addAction(DFMExtAction *action) = 0;
    virtual bool insertAction(DFMExtAction *before, DFMExtAction *action) = 0;

    virtual DFMExtAction *menuAction() const = 0;
    virtual std::list<DFMExtAction *> actions() const = 0;
protected:
    DFMExtMenu::TriggeredFunc triggeredFunc;
    DFMExtMenu::HoveredFunc hoveredFunc;
    DFMExtMenu::DeletedFunc deletedFunc;
};

END_DFMEXT_NAMESPACE

#endif // DFMEXTMENUPRIVATE_H
