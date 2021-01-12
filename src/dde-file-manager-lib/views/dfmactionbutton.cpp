/**
 * Copyright (C) 2017 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/
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
