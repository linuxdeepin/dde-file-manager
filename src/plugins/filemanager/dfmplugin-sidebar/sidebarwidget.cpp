/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#include "sidebarwidget.h"
#include "sidebarview.h"
#include "sidebarmodel.h"
#include "sidebaritem.h"
#include "sidebaritemdelegate.h"

#include <QApplication>
#include <QVBoxLayout>
#include <QScrollBar>

DFMBASE_USE_NAMESPACE

SideBarWidget::SideBarWidget(QFrame *parent)
    : AbstractFrame(parent),
      sidebarView(new SideBarView(this)),
      sidebarModel(new SideBarModel(this))
{
    initializeUi();
    initConnect();
}

void SideBarWidget::setCurrentUrl(const QUrl &url)
{
    auto model = sidebarView->model();
    sidebarView->selectionModel()->clearSelection();
    for (int i = 0; i < model->rowCount(); i++) {
        auto item = sidebarView->model()->itemFromIndex(i);
        if (item->url() == url) {
            sidebarView->selectionModel()->select(model->index(i, 0), QItemSelectionModel::SelectCurrent);
            return;
        }
        if (item->url().scheme() == url.scheme()) {
            sidebarView->selectionModel()->select(model->index(i, 0), QItemSelectionModel::SelectCurrent);
            return;
        }
    }
}

void SideBarWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::PaletteChange) {
        sidebarView->setBackgroundType(DStyledItemDelegate::BackgroundType(DStyledItemDelegate::RoundedBackground
                                                                           | DStyledItemDelegate::NoNormalState));
        sidebarView->setItemSpacing(0);
    }

    return QWidget::changeEvent(event);
}

QAbstractItemView *SideBarWidget::view()
{
    return sidebarView;
}

int SideBarWidget::addItem(SideBarItem *item)
{
    return sidebarModel->appendRow(item);
}

bool SideBarWidget::insertItem(const int index, SideBarItem *item)
{
    return sidebarModel->insertRow(index, item);
}

bool SideBarWidget::removeItem(SideBarItem *item)
{
    return sidebarModel->removeRow(item);
}

void SideBarWidget::onItemClicked(const QModelIndex &index)
{
    SideBarItem *item = sidebarModel->itemFromIndex(index);
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    QUrl url = qvariant_cast<QUrl>(item->data(SideBarItem::Roles::ItemUrlRole));
    if (!url.isEmpty())
        Q_EMIT clickedItemUrl(url);
}

void SideBarWidget::customContextMenuCall(const QPoint &pos)
{
    // 拿到Item对应的QUrl
    const QUrl &url = sidebarView->urlAt(pos);
    // 相对坐标转全局坐标
    const QPoint &globalPos = sidebarView->mapToGlobal(pos);

    Q_EMIT customContextMenu(url, globalPos);
}

void SideBarWidget::initializeUi()
{
    QVBoxLayout *vlayout = new QVBoxLayout(this);
    vlayout->addWidget(sidebarView);
    vlayout->setMargin(0);
    vlayout->setSpacing(0);

    sidebarView->setModel(sidebarModel);
    sidebarView->setItemDelegate(new SideBarItemDelegate(sidebarView));
    sidebarView->setViewportMargins(10, 0, sidebarView->verticalScrollBar()->sizeHint().width(), 0);
    sidebarView->setContextMenuPolicy(Qt::CustomContextMenu);
    sidebarView->setFrameShape(QFrame::Shape::NoFrame);
    sidebarView->setAutoFillBackground(true);
    sidebarView->setBackgroundType(DStyledItemDelegate::BackgroundType(DStyledItemDelegate::RoundedBackground
                                                                       | DStyledItemDelegate::NoNormalState));
    sidebarView->setItemSpacing(0);

    setMaximumWidth(200);
    setFocusProxy(sidebarView);
}

void SideBarWidget::initConnect()
{
    connect(sidebarView, &SideBarView::clicked,
            this, &SideBarWidget::clickedItemIndex);

    connect(sidebarView, &SideBarView::clicked,
            this, &SideBarWidget::onItemClicked,
            Qt::UniqueConnection);

    connect(sidebarView, &SideBarView::customContextMenuRequested,
            this, &SideBarWidget::customContextMenuCall);
}
