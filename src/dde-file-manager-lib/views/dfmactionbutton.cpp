// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfmactionbutton.h"

#include <QAction>
#include <QApplication>

DFMActionButton::DFMActionButton(QWidget *parent)
    : QToolButton(parent)
{
    setMouseTracking(true);
}

void DFMActionButton::setAction(QAction *action)
{
    auto onActionChanged = [this, action] {
        setVisible(action->isVisible());
        setEnabled(action->isEnabled());
        setCheckable(action->isCheckable());
        setChecked(action->isChecked());
        setIcon(action->icon());
    };

    onActionChanged();
    connect(action, &QAction::changed, this, onActionChanged);
    connect(this, &DFMActionButton::clicked, action, &QAction::trigger);
    connect(action, &QAction::triggered, this, &DFMActionButton::setChecked);

    setDefaultAction(action);
}

QAction *DFMActionButton::action() const
{
    return actions().value(0);
}
