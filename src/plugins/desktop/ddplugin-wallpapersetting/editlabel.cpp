// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "editlabel.h"

#include <QKeyEvent>

DDP_WALLPAERSETTING_USE_NAMESPACE

EditLabel::EditLabel(QWidget *parent)
    : QLabel(parent)
{
}

void EditLabel::mousePressEvent(QMouseEvent *event)
{
    if (Qt::MouseButton::LeftButton == event->button()) {
        emit editLabelClicked();
    } else {
        QLabel::mousePressEvent(event);
    }
}
