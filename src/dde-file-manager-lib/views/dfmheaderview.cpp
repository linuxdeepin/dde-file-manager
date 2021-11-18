/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     Gary Wang <wzc782970009@gmail.com>
 *
 * Maintainer: Gary Wang <wangzichong@deepin.com>
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
