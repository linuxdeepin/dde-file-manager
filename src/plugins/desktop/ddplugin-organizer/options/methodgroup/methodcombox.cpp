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

#include "methodcombox.h"

using namespace ddplugin_organizer;
DWIDGET_USE_NAMESPACE

MethodComBox::MethodComBox(const QString &title, QWidget *parent)
    : EntryWidget(new QLabel(title), new DComboBox(), parent)
{
    label = qobject_cast<QLabel* >(leftWidget);
    label->setParent(this);
    label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    comboBox = qobject_cast<DComboBox *>(rightWidget);
    comboBox->setParent(this);
    comboBox->setFixedSize(198, 36);

    //Enable the combo box
    comboBox->setVisible(false);

    connect(comboBox, (void (DComboBox::*)(int))&DComboBox::currentIndexChanged, this, &MethodComBox::methodChanged);
}

void MethodComBox::initCheckBox()
{
    // -1 Not open
    //comboBox->addItem(tr("Custom")); // todo 文案

    // just use type at present
    // kType 0
    comboBox->addItem(tr("Type"));

    // Not open
//    comboBox->addItem(tr("Time accessed"));
//    comboBox->addItem(tr("Time modified"));
//    comboBox->addItem(tr("Time created"));
}

void MethodComBox::setCurrentMethod(int idx)
{
    blockSignals(true);
#if 0 // Not open
    // increase 1
    idx++;
    if (idx <= 0)
        comboBox->setCurrentIndex(0);
    else if (idx < comboBox->count()) {
        comboBox->setCurrentIndex(idx);
    }
#else // using fixed index for type
    comboBox->setCurrentIndex(0);
#endif
    blockSignals(false);
}

int MethodComBox::currentMethod()
{
#if 0
    // mins one
    return comboBox->currentIndex() - 1;
#else
    return 0; // just type
#endif
}
