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
#ifndef DFMEXTACTION_H
#define DFMEXTACTION_H

#include "dfm-extension-global.h"
#include "dfmextmenuproxy.h"

#include <functional>
#include <string>

BEGEN_DFMEXT_NAMESPACE

class DFMExtMenu;
class DFMExtActionPrivate;
class DFMExtAction
{
    friend class DFMExtActionPrivate;
public:
    using TriggeredFunc = std::function<void (DFMExtAction *, bool)>;
    using HoveredFunc = std::function<void (DFMExtAction *)>;
    using DeletedFunc = std::function<void (DFMExtAction *)>;

public:
    void setIcon(const std::string &icon);
    std::string icon() const;

    void setText(const std::string &text);
    std::string text() const;

    void setToolTip(const std::string &tip);
    std::string toolTip() const;

    void setMenu(DFMExtMenu *menu);
    DFMExtMenu *menu() const;

    void setSeparator(bool b);
    bool isSeparator() const;

    void setCheckable(bool b);
    bool isCheckable() const;

    void setChecked(bool b);
    bool isChecked() const;

    void setEnabled(bool b);
    bool isEnabled() const;

    DFM_FAKE_VIRTUAL void triggered(DFMExtAction *self, bool checked = false);
    DFM_FAKE_VIRTUAL void hovered(DFMExtAction *self);
    DFM_FAKE_VIRTUAL void deleted(DFMExtAction *self);

public:
    void registerTriggered(const TriggeredFunc &func);
    void registerHovered(const HoveredFunc &func);
    void registerDeleted(const DeletedFunc &func);

protected:
    explicit DFMExtAction(DFMExtActionPrivate *d_ptr);
    virtual ~DFMExtAction();
    DFMExtActionPrivate *d;
};

END_DFMEXT_NAMESPACE

#endif // DFMEXTACTION_H
