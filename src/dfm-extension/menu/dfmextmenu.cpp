// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "private/dfmextmenuprivate.h"

#include <dfm-extension/menu/dfmextmenu.h>

#include <cassert>

USING_DFMEXT_NAMESPACE

DFMExtMenu::DFMExtMenu(DFMExtMenuPrivate *d_ptr)
    : d(d_ptr)
{
    assert(d);
}

DFMExtMenu::~DFMExtMenu()
{
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
