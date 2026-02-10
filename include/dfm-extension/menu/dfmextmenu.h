// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DFMEXTMENU_H
#define DFMEXTMENU_H

#include <dfm-extension/dfm-extension-global.h>

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

    ~DFMExtMenu();

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
    DFMExtMenuPrivate *d;
};

END_DFMEXT_NAMESPACE

#endif   // DFMEXTMENU_H
