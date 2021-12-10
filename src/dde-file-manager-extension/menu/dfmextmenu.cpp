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
#include "dfmextmenu.h"
#include "private/dfmextmenuprivate.h"

#include <assert.h>

USING_DFMEXT_NAMESPACE

DFMExtMenu::DFMExtMenu(DFMExtMenuPrivate *d_ptr)
    : d(d_ptr)
{
    assert(d);
}

DFMExtMenu::~DFMExtMenu()
{
    if (d->deletedFunc)
        d->deletedFunc(this);

    delete d;
    d = nullptr;
}

std::string DFMExtMenu::title() const
{
    return d->title();
}

void DFMExtMenu::setTitle(const std::string &title)
{
    d->setTitle(title);
}

std::string DFMExtMenu::icon() const
{
    return d->icon();
}

void DFMExtMenu::setIcon(const std::string &iconName)
{
    d->setIcon(iconName);
}

bool DFMExtMenu::addAction(DFMExtAction *action)
{
    return d->addAction(action);
}

bool DFMExtMenu::insertAction(DFMExtAction *before, DFMExtAction *action)
{
    return d->insertAction(before, action);
}

DFMExtAction *DFMExtMenu::menuAction() const
{
    return d->menuAction();
}

std::list<DFMExtAction *> DFMExtMenu::actions() const
{
    return d->actions();
}

void DFMExtMenu::triggered(DFMExtAction *action)
{
    if (d->triggeredFunc)
        d->triggeredFunc(action);
}

void DFMExtMenu::hovered(DFMExtAction *action)
{
    if (d->hoveredFunc)
        d->hoveredFunc(action);
}

void DFMExtMenu::deleted(DFMExtMenu *self)
{
    if (d->deletedFunc)
        d->deletedFunc(self);
}

void dfmext::DFMExtMenu::registerTriggered(const dfmext::DFMExtMenu::TriggeredFunc &func)
{
    d->triggeredFunc = func;
}

void dfmext::DFMExtMenu::registerHovered(const dfmext::DFMExtMenu::HoveredFunc &func)
{
    d->hoveredFunc = func;
}

void DFMExtMenu::registerDeleted(const DFMExtMenu::DeletedFunc &func)
{
    d->deletedFunc = func;
}
