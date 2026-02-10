// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "editlabel.h"

#include <QKeyEvent>

DDP_WALLPAERSETTING_USE_NAMESPACE

EditLabel::EditLabel(QWidget *parent)
    : QLabel(parent)
{
}

void EditLabel::setHotZoom(const QRect &rect)
{
    hotZoom = rect;
}

void EditLabel::mousePressEvent(QMouseEvent *event)
{
    if (Qt::MouseButton::LeftButton == event->button()) {
        if (!hotZoom.isValid() || hotZoom.contains(event->pos())) {
            event->accept();
            emit editLabelClicked();
            return;
        }
    }

    QLabel::mousePressEvent(event);
}
