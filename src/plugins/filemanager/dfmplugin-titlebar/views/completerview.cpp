// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "views/completerview.h"

#include <QScrollBar>

using namespace dfmplugin_titlebar;
CompleterView::CompleterView(QWidget *parent)
    : DListView(parent)
{
    overrideWindowFlags(Qt::Tool /*| Qt::WindowDoesNotAcceptFocus*/);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setViewportMargins(0, kItemMargin, -verticalScrollBar()->sizeHint().width(), kItemMargin);
    setMouseTracking(true);
    //解决bug19609文件管理器中，文件夹搜索功能中输入法在输入过程中忽然失效然后恢复，设置这个属性listview就可以拥有地址兰的输入法
    setAttribute(Qt::WA_InputMethodEnabled);
}

void CompleterView::keyPressEvent(QKeyEvent *e)
{
    return QListView::keyPressEvent(e);
}

void CompleterView::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous);
    emit listCurrentChanged(current);
    QListView::currentChanged(current, previous);
}

void CompleterView::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(deselected);
    emit listSelectionChanged(selected);
    QListView::selectionChanged(selected, deselected);
}
