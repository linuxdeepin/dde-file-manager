// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "switchwidget.h"

#include <DPalette>

#include <QPainter>

DWIDGET_USE_NAMESPACE
using namespace ddplugin_organizer;

SwitchWidget::SwitchWidget(const QString &title, QWidget *parent)
    : EntryWidget(new QLabel(title), new DSwitchButton(), parent)
{
    label = qobject_cast<QLabel *>(leftWidget);
    label->setParent(this);
    label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    switchBtn = qobject_cast<DSwitchButton *>(rightWidget);
    switchBtn->setParent(this);

    connect(switchBtn, &DSwitchButton::toggled, this, &SwitchWidget::checkedChanged);
}

void SwitchWidget::setChecked(bool checked)
{
    blockSignals(true);
    switchBtn->setChecked(checked);
    blockSignals(false);
}

bool SwitchWidget::checked() const
{
    return switchBtn->isChecked();
}

void SwitchWidget::setTitle(const QString &title)
{
    label->setText(title);
    label->setWordWrap(true);
    label->adjustSize();
}
