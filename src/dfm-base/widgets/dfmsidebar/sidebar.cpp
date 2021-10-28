/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#include "sidebar.h"
#include "sidebaritemdelegate.h"
#include "sidebaritem.h"
#include "private/sidebar_p.h"
//#include "base/singleton.hpp"

#include <DApplicationHelper>

#include <QScrollBar>
#include <QVBoxLayout>
#include <QDebug>
#include <QMenu>
#include <QtConcurrent>
#include <QApplication>

#include <algorithm>

DFMBASE_BEGIN_NAMESPACE

SideBarPrivate::SideBarPrivate(SideBar *qq)
    : QObject(qq)
    , q(qq)
    , sidebarView(new SideBarView(qq))
    , sidebarModel(new SideBarModel(qq))
{
    QObject::connect(sidebarView, &SideBarView::clicked,
                     q, &SideBar::clickedItemIndex,
                     Qt::UniqueConnection);

    QObject::connect(sidebarView, &SideBarView::clicked,
                     this, &SideBarPrivate::onItemClicked,
                     Qt::UniqueConnection);

    QVBoxLayout *vlayout = new QVBoxLayout(q);
    vlayout->addWidget(sidebarView);
    vlayout->setMargin(0);
    vlayout->setSpacing(0);

    sidebarView->setModel(sidebarModel);
    sidebarView->setItemDelegate(new SideBarItemDelegate(sidebarView));
    sidebarView->setViewportMargins(10, 0, sidebarView->verticalScrollBar()->sizeHint().width(), 0);
    sidebarView->setContextMenuPolicy(Qt::CustomContextMenu);
    sidebarView->setFrameShape(QFrame::Shape::NoFrame);
    sidebarView->setAutoFillBackground(true);
    sidebarView->setBackgroundType(DStyledItemDelegate::BackgroundType
                                   (DStyledItemDelegate::RoundedBackground
                                    | DStyledItemDelegate::NoNormalState));
    sidebarView->setItemSpacing(0);

    q->setMaximumWidth(200);
    q->setFocusProxy(sidebarView);
}

SideBar::SideBar(QWidget *parent)
    : QWidget(parent)
    , d(new SideBarPrivate(this))
{

}

SideBar::~SideBar()
{

}

QAbstractItemView *SideBar::view()
{
    return d->sidebarView;
}

void SideBar::setCurrentUrl(const QUrl &url)
{
    auto model = d->sidebarView->model();
    d->sidebarView->selectionModel()->clearSelection();
    for (int i = 0; i< model->rowCount(); i++ ) {
        auto item = d->sidebarView->model()->itemFromIndex(i);
        if (item->url() == url) {
            d->sidebarView->selectionModel()->select(model->index(i,0),QItemSelectionModel::SelectCurrent);
            return;
        }
        if (item->url().scheme() == url.scheme()) {
            d->sidebarView->selectionModel()->select(model->index(i,0),QItemSelectionModel::SelectCurrent);
            return;
        }
    }
}

int SideBar::addItem(SideBarItem *item)
{
    return d->sidebarModel->appendRow(item);
}

bool SideBar::insertItem(const int index, SideBarItem *item)
{
    return d->sidebarModel->insertRow(index, item);
}

bool SideBar::removeItem(SideBarItem *item)
{
    return d->sidebarModel->removeRow(item);
}

void SideBarPrivate::onItemClicked(const QModelIndex &index)
{
    qInfo() << "index" << index;
    SideBarItem *item = sidebarModel->itemFromIndex(index);
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    QUrl url = qvariant_cast<QUrl>(item->data(SideBarItem::Roles::ItemUrlRole));
    if (!url.isEmpty())
        Q_EMIT q->clickedItemUrl(url);
}

void SideBar::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::PaletteChange) {
        d->sidebarView->setBackgroundType(DStyledItemDelegate::BackgroundType
                                          (DStyledItemDelegate::RoundedBackground
                                           | DStyledItemDelegate::NoNormalState));
        d->sidebarView->setItemSpacing(0);
    }

    return QWidget::changeEvent(event);
}

DFMBASE_END_NAMESPACE
