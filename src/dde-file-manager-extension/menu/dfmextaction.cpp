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
#include "dfmextaction.h"
#include "private/dfmextactionprivate.h"

#include <assert.h>

USING_DFMEXT_NAMESPACE

DFMExtAction::DFMExtAction(DFMExtActionPrivate *d_ptr): d(d_ptr)
{
    assert(d);
}

DFMExtAction::~DFMExtAction()
{
    delete d;
}

void DFMExtAction::setIcon(const std::string &icon)
{
    d->setIcon(icon);
}

std::string DFMExtAction::icon() const
{
    return d->icon();
}

void DFMExtAction::setText(const std::string &text)
{
    d->setText(text);
}

std::string DFMExtAction::text() const
{
    return d->text();
}

void DFMExtAction::setToolTip(const std::string &tip)
{
    d->setToolTip(tip);
}

std::string DFMExtAction::toolTip() const
{
    return d->toolTip();
}

void DFMExtAction::setMenu(DFMExtMenu *menu)
{
    d->setMenu(menu);
}

DFMExtMenu *DFMExtAction::menu() const
{
    return d->menu();
}

void DFMExtAction::setSeparator(bool b)
{
    d->setSeparator(b);
}

bool DFMExtAction::isSeparator() const
{
    return d->isSeparator();
}

void DFMExtAction::setCheckable(bool b)
{
    d->setCheckable(b);
}

bool DFMExtAction::isCheckable() const
{
    return d->isCheckable();
}

void DFMExtAction::setChecked(bool b)
{
    d->setChecked(b);
}

bool DFMExtAction::isChecked() const
{
    return d->isChecked();
}

void DFMExtAction::setEnabled(bool b)
{
    return d->setEnabled(b);
}

bool DFMExtAction::isEnabled() const
{
    return d->isEnabled();
}

void dfmext::DFMExtAction::triggered(DFMExtAction *self, bool checked)
{
    if (d->triggeredFunc)
        d->triggeredFunc(self, checked);
}

void dfmext::DFMExtAction::hovered(DFMExtAction *self)
{
    if (d->hoveredFunc)
        d->hoveredFunc(self);
}

void DFMExtAction::deleted(DFMExtAction *self)
{
    if (d->deletedFunc)
        d->deletedFunc(self);
}

void DFMExtAction::registerTriggered(const DFMExtAction::TriggeredFunc &func)
{
    d->triggeredFunc = func;
}

void DFMExtAction::registerHovered(const DFMExtAction::HoveredFunc &func)
{
    d->hoveredFunc = func;
}

void DFMExtAction::registerDeleted(const DFMExtAction::DeletedFunc &func)
{
    d->deletedFunc = func;
}

