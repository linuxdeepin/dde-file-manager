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
#ifndef DFMEXTACTIONPRIVATE_H
#define DFMEXTACTIONPRIVATE_H

#include "dfm-extension-global.h"
#include "menu/dfmextaction.h"

#include <functional>
#include <string>

BEGEN_DFMEXT_NAMESPACE

class DFMExtMenu;
class DFMExtActionPrivate
{
    friend class DFMExtAction;
public:
    explicit DFMExtActionPrivate();
    virtual ~DFMExtActionPrivate();

    virtual void setIcon(const std::string &iconName) = 0;
    virtual std::string icon() const = 0;

    virtual void setText(const std::string &text) = 0;
    virtual std::string text() const = 0;

    virtual void setToolTip(const std::string &tip) = 0;
    virtual std::string toolTip() const = 0;

    virtual void setMenu(DFMExtMenu *menu) = 0;
    virtual DFMExtMenu *menu() const = 0;

    virtual void setSeparator(bool b) = 0;
    virtual bool isSeparator() const = 0;

    virtual void setCheckable(bool b) = 0;
    virtual bool isCheckable() const = 0;

    virtual void setChecked(bool b) = 0;
    virtual bool isChecked() const = 0;

    virtual void setEnabled(bool) = 0;
    virtual bool isEnabled() const = 0;
protected:
    DFMExtAction::TriggeredFunc triggeredFunc;
    DFMExtAction::HoveredFunc hoveredFunc;
    DFMExtAction::DeletedFunc deletedFunc;
};

END_DFMEXT_NAMESPACE
#endif // DFMEXTACTIONPRIVATE_H
