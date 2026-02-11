// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "entrywidget.h"

#include <QHBoxLayout>

using namespace ddplugin_organizer;

EntryWidget::EntryWidget(QWidget *left, QWidget *right, QWidget *parent)
    : ContentBackgroundWidget(parent), leftWidget(left), rightWidget(right)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setSpacing(10);
    layout->setContentsMargins(10, 0, 10, 0);
    setLayout(layout);

    if (leftWidget && rightWidget) {
        layout->addWidget(leftWidget, 0, Qt::AlignLeft);
        layout->addWidget(rightWidget, 0, Qt::AlignRight);
    } else if (leftWidget) {   // only lelf
        layout->addWidget(leftWidget, 1, Qt::AlignLeft);
    } else if (rightWidget) {   // only right
        layout->addWidget(rightWidget, 1, Qt::AlignRight);
    }
}
