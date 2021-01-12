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
#include "dcompleterlistview.h"
#include "accessibility/ac-lib-file-manager.h"

#include <QDebug>
#include <QScrollBar>

#include <dtkwidget_global.h>

#include <QApplication>
#include <QDebug>


DWIDGET_USE_NAMESPACE

DFM_BEGIN_NAMESPACE

DCompleterListView::DCompleterListView(QWidget *parent)
    : QListView(parent)
{
    AC_SET_OBJECT_NAME(this, AC_COMPUTER_COMPLETER_LIST_VIEW);
    AC_SET_ACCESSIBLE_NAME(this, AC_COMPUTER_COMPLETER_LIST_VIEW);

    overrideWindowFlags(Qt::Tool /*| Qt::WindowDoesNotAcceptFocus*/);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setViewportMargins(0, 0, -verticalScrollBar()->sizeHint().width(), 0);
    setMouseTracking(true);
}
//解决bug19609文件管理器中，文件夹搜索功能中输入法在输入过程中忽然失效然后恢复
void DCompleterListView::keyPressEvent(QKeyEvent *e)
{
    return QListView::keyPressEvent(e);
}

void DCompleterListView::showMe()
{
    QListView::show();
}
//隐藏、停止计时器、释放鼠标
void DCompleterListView::hideMe()
{
    QListView::hide();
}

void DCompleterListView::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous);
    emit listCurrentChanged(current);
    QListView::currentChanged(current, previous);
}

void DCompleterListView::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(deselected);
    emit listSelectionChanged(selected);
    QListView::selectionChanged(selected, deselected);
}

DFM_END_NAMESPACE
