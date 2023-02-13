// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMEXTACTIONPRIVATE_H
#define DFMEXTACTIONPRIVATE_H

#include <dfm-extension/dfm-extension-global.h>
#include <dfm-extension/menu/dfmextaction.h>

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
#endif   // DFMEXTACTIONPRIVATE_H
