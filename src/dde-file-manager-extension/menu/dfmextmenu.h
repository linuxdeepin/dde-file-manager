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
#ifndef DFMEXTMENU_H
#define DFMEXTMENU_H

#include "dfm-extension-global.h"

#include <functional>
#include <string>
#include <list>

BEGEN_DFMEXT_NAMESPACE

class DFMExtAction;
class DFMExtMenuPrivate;
class DFMExtMenu
{
    friend class DFMExtMenuPrivate;

public:
    using TriggeredFunc = std::function<void(DFMExtAction *action)>;
    using HoveredFunc = std::function<void(DFMExtAction *action)>;
    using DeletedFunc = std::function<void(DFMExtMenu *)>;

public:
    std::string title() const;
    void setTitle(const std::string &title);

    std::string icon() const;
    void setIcon(const std::string &iconName);

    bool addAction(DFMExtAction *action);
    bool insertAction(DFMExtAction *before, DFMExtAction *action);

    DFMExtAction *menuAction() const;
    std::list<DFMExtAction *> actions() const;

    DFM_FAKE_VIRTUAL void triggered(DFMExtAction *action);
    DFM_FAKE_VIRTUAL void hovered(DFMExtAction *action);
    DFM_FAKE_VIRTUAL void deleted(DFMExtMenu *self);

public:
    void registerTriggered(const TriggeredFunc &func);
    void registerHovered(const HoveredFunc &func);
    void registerDeleted(const DeletedFunc &func);

protected:
    explicit DFMExtMenu(DFMExtMenuPrivate *d_ptr);
    virtual ~DFMExtMenu();
    DFMExtMenuPrivate *d;
};

END_DFMEXT_NAMESPACE

#endif   // DFMEXTMENU_H
