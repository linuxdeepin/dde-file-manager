/*
 * Copyright (C) 2017 ~ 2020 Deepin Technology Co., Ltd.
 *
 * Author:     liuyang <liuyang_cm@deepin.com>
 *
 * Maintainer: liuyang <liuyang_cm@deepin.com>
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

#include "dfmsplitter.h"
#include "accessibility/ac-lib-file-manager.h"

#include <QDebug>
#include <QGuiApplication>
#include <QSizePolicy>

DFMSplitterHandle::DFMSplitterHandle(Qt::Orientation orientation, QSplitter *parent):
    QSplitterHandle(orientation, parent)
{

}

void DFMSplitterHandle::enterEvent(QEvent *)
{
    QGuiApplication::setOverrideCursor(orientation() == Qt::Horizontal ? Qt::SizeHorCursor : Qt::SizeVerCursor);
}

void DFMSplitterHandle::leaveEvent(QEvent *)
{
    QGuiApplication::restoreOverrideCursor();
}

// ---------- Item Get Border Line ----------

DFMSplitter::DFMSplitter(Qt::Orientation orientation, QWidget *parent)
    :QSplitter(orientation, parent)
{
    AC_SET_OBJECT_NAME(this, AC_DM_SPLITTER);
    AC_SET_ACCESSIBLE_NAME(this, AC_DM_SPLITTER);
}

void DFMSplitter::moveSplitter(int pos, int index)
{
    return QSplitter::moveSplitter(pos, index);
}

QSplitterHandle *DFMSplitter::createHandle()
{
    return new DFMSplitterHandle(orientation(), this);
}
