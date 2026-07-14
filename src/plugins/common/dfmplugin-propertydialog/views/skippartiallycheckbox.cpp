// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "skippartiallycheckbox.h"

using namespace dfmplugin_propertydialog;
DWIDGET_USE_NAMESPACE

SkipPartiallyCheckBox::SkipPartiallyCheckBox(QWidget *parent)
    : DCheckBox(parent)
{
    setTristate(true);
}

void SkipPartiallyCheckBox::nextCheckState()
{
    Qt::CheckState current = checkState();
    if (current == Qt::Unchecked) {
        setCheckState(Qt::Checked);
    } else {
        setCheckState(Qt::Unchecked);
    }
}
