// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfmheaderview.h"

DFMHeaderView::DFMHeaderView(Qt::Orientation orientation, QWidget *parent)
    : QHeaderView(orientation, parent)
{
    setSectionsMovable(true);
    setFirstSectionMovable(false);
}

QSize DFMHeaderView::sizeHint() const
{
    QSize size = QHeaderView::sizeHint();

    size.setWidth(length());

    return size;
}

void DFMHeaderView::mouseReleaseEvent(QMouseEvent *e)
{
    emit mouseReleased();

    return QHeaderView::mouseReleaseEvent(e);
}

void DFMHeaderView::resizeEvent(QResizeEvent *e)
{
    emit viewResized();

    return QHeaderView::resizeEvent(e);
}
