// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "splitter.h"

#include <QDebug>
#include <QGuiApplication>
#include <QSizePolicy>

namespace dfmbase {

SplitterHandle::SplitterHandle(Qt::Orientation orientation, QSplitter *parent)
    : QSplitterHandle(orientation, parent)
{
}
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
void SplitterHandle::enterEvent(QEnterEvent *)
#else
void SplitterHandle::enterEvent(QEvent *)
#endif
{
    QGuiApplication::setOverrideCursor(orientation() == Qt::Horizontal ? Qt::SizeHorCursor : Qt::SizeVerCursor);
}

void SplitterHandle::leaveEvent(QEvent *)
{
    QGuiApplication::restoreOverrideCursor();
}

// ---------- Item Get Border Line ----------

Splitter::Splitter(Qt::Orientation orientation, QWidget *parent)
    : QSplitter(orientation, parent)
{
}

void Splitter::moveSplitter(int pos, int index)
{
    return QSplitter::moveSplitter(pos, index);
}

QSplitterHandle *Splitter::createHandle()
{
    return new SplitterHandle(orientation(), this);
}

}
