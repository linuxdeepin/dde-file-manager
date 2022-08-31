/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangchunlin<wangchunlin@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
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
#include "switchwidget.h"

#include <DPalette>
#include <DApplicationHelper>

#include <QPainter>

DWIDGET_USE_NAMESPACE
using namespace ddplugin_organizer;

SwitchWidget::SwitchWidget(const QString &title, QWidget *parent)
    : EntryWidget(new QLabel(title), new DSwitchButton(), parent)
{
    label = qobject_cast<QLabel* >(leftWidget);
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
