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
//    setFocusPolicy(Qt::NoFocus);
    overrideWindowFlags(Qt::Tool /*| Qt::WindowDoesNotAcceptFocus*/);
    setFocusPolicy (Qt::StrongFocus);
    setAttribute (Qt::WA_InputMethodEnabled);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setViewportMargins(0, 0, -verticalScrollBar()->sizeHint().width(), 0);
    setMouseTracking(true);

    //当listview设在popup属性时，lineedit的输入法没法传递当前的listview
    //所以在DFMAddressBar中将当前的 listview设置了Sheet属性，可以传递输入法
    //但是失去了popup属性，不能在点击空白出自动隐藏listview
    //下面代码主要是实现“点击空白处，当前listview自动隐藏的功能
    //显示自动匹配的界面就启动计时器
    m_timer = new QTimer(this);
    m_timer->setInterval(1);
    connect(this,&QListView::hide,this,&DCompleterListView::hideMe);
    connect(m_timer,&QTimer::timeout,this,[this,parent](){
        //判断鼠标位置在当前的view内就不截获鼠标，不在就截获鼠标
        QPoint point = this->mapFromGlobal(QApplication::overrideCursor()->pos());
        if (!isHidden() && isVisible()) {
            m_bshow = true;
        }
        if (m_bshow && (isHidden() || !isVisible()))
        {
            hideMe();
            return;
        }
        //获取主窗口位置，位置发生改变就隐藏
        if (m_bshow) {
            QWidget *parentnew = parent;
            QWidget *window = nullptr;
            while (parentnew){
                window = parentnew;
                parentnew = parentnew->parentWidget();
            }
            if (window) {
                if (m_windowpos.isNull()) {
                    m_windowpos = window->pos();
                }
                else {
                    if (m_windowpos != window->pos()) {
                        hideMe();
                        return;
                    }
                }
                if (window != qApp->activeWindow()) {
                    hideMe();
                    return;
                }
            }
        }
        if (rect().contains(point))
        {
            if (m_bgrabmouse) {
                releaseMouse();
                m_bgrabmouse = false;
            }
        }
        else {
            if (!m_bgrabmouse) {
                grabMouse();
                m_bgrabmouse = true;
            }
        }
    });
}
//解决bug19609文件管理器中，文件夹搜索功能中输入法在输入过程中忽然失效然后恢复
void DCompleterListView::keyPressEvent(QKeyEvent *e)
{
    return QListView::keyPressEvent(e);
}

void DCompleterListView::showMe()
{
    m_timer->start();
    m_bshow = false;
    m_bgrabmouse = false;
    m_windowpos = QPoint();
    QListView::show();
}
//隐藏、停止计时器、释放鼠标
void DCompleterListView::hideMe()
{
    m_timer->stop();
    releaseMouse();
    m_bshow = false;
    m_windowpos = QPoint();
    m_bgrabmouse = false;
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
