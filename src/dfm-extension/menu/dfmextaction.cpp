// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "private/dfmextactionprivate.h"

#include <dfm-extension/menu/dfmextaction.h>

#include <assert.h>

USING_DFMEXT_NAMESPACE

DFMExtAction::DFMExtAction(DFMExtActionPrivate *d_ptr)
    : d(d_ptr)
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
