// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
