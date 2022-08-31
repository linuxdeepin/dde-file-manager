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

#include "entrywidget.h"

#include <QHBoxLayout>

using namespace ddplugin_organizer;

EntryWidget::EntryWidget(QWidget *left, QWidget *right, QWidget *parent)
    : ContentBackgroundWidget(parent)
    , leftWidget(left)
    , rightWidget(right)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setSpacing(10);
    layout->setContentsMargins(10, 0, 10, 0);
    setLayout(layout);

    if (leftWidget && rightWidget) {
        layout->addWidget(leftWidget, 0, Qt::AlignLeft);
        layout->addWidget(rightWidget, 0, Qt::AlignRight);
    } else if (leftWidget) { // only lelf
        layout->addWidget(leftWidget, 1, Qt::AlignLeft);
    } else if (rightWidget) { // only right
        layout->addWidget(rightWidget, 1, Qt::AlignRight);
    }
}

