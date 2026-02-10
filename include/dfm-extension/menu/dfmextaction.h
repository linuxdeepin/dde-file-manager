// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DFMEXTACTION_H
#define DFMEXTACTION_H

#include <dfm-extension/dfm-extension-global.h>
#include <dfm-extension/menu/dfmextmenuproxy.h>

#include <functional>
#include <string>

BEGEN_DFMEXT_NAMESPACE

class DFMExtMenu;
class DFMExtActionPrivate;
class DFMExtAction
{
    friend class DFMExtActionPrivate;

public:
    using TriggeredFunc = std::function<void(DFMExtAction *, bool)>;
    using HoveredFunc = std::function<void(DFMExtAction *)>;
    using DeletedFunc = std::function<void(DFMExtAction *)>;

    ~DFMExtAction();

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
    DFMExtActionPrivate *d;
};

END_DFMEXT_NAMESPACE

#endif   // DFMEXTACTION_H
