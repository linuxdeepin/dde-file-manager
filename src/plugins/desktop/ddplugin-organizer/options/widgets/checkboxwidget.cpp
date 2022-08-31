/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
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

#include "checkboxwidget.h"

DWIDGET_USE_NAMESPACE
using namespace ddplugin_organizer;

CheckBoxWidget::CheckBoxWidget(const QString &text, QWidget *parent)
    : EntryWidget(new DCheckBox(text), nullptr, parent)
{
    checkBox = qobject_cast<DCheckBox* >(leftWidget);
    connect(checkBox, &QCheckBox::stateChanged, this, [this](int stat){
        emit chenged(stat == Qt::Checked);
    });
}

void CheckBoxWidget::setChecked(bool checked)
{
    blockSignals(true);
    checkBox->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
    blockSignals(false);
}

bool CheckBoxWidget::checked() const
{
    return checkBox->checkState() == Qt::Checked;
}
