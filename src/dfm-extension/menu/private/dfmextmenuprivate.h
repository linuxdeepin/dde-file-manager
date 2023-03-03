// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMEXTMENUPRIVATE_H
#define DFMEXTMENUPRIVATE_H

#include <dfm-extension/dfm-extension-global.h>
#include <dfm-extension/menu/dfmextmenu.h>

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

#endif   // DFMEXTMENUPRIVATE_H
