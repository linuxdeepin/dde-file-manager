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
#include "models/sidebarmodel.h"
#include "views/sidebarview.h"
#include "views/sidebaritem.h"
#include "views/sidebaritemdelegate.h"
#include "events/sidebareventcaller.h"
#include "utils/sidebarhelper.h"

#include "services/filemanager/sidebar/sidebar_defines.h"
#include "dfm-base/utils/systempathutil.h"

#include <QApplication>
#include <QVBoxLayout>
#include <QScrollBar>
#include <QThread>

DPSIDEBAR_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DSB_FM_USE_NAMESPACE

SideBarWidget::SideBarWidget(QFrame *parent)
    : AbstractFrame(parent),
      sidebarView(new SideBarView(this)),
      sidebarModel(new SideBarModel(this)),
      currentGroups { SideBar::DefaultGroup::kCommon, SideBar::DefaultGroup::kDevice,
                      SideBar::DefaultGroup::kBookmark, SideBar::DefaultGroup::kNetwork,
                      SideBar::DefaultGroup::kTag, SideBar::DefaultGroup::kOther }
{
    initializeUi();
    initDefaultModel();
    initConnect();
}

void SideBarWidget::setCurrentUrl(const QUrl &url)
{
    sidebarUrl = url;
    auto model = sidebarView->model();
    sidebarView->selectionModel()->clearSelection();
    for (int i = 0; i < model->rowCount(); i++) {
        auto item = sidebarView->model()->itemFromIndex(i);
        if (item->url() == sidebarUrl) {
            sidebarView->selectionModel()->select(model->index(i, 0), QItemSelectionModel::SelectCurrent);
            return;
        }
    }
}

QUrl SideBarWidget::currentUrl() const
{
    return sidebarUrl;
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
    Q_ASSERT(qApp->thread() == QThread::currentThread());
    // TODO(zhangs): custom group
    return sidebarModel->appendRow(item);
}

bool SideBarWidget::insertItem(const int index, SideBarItem *item)
{
    Q_ASSERT(qApp->thread() == QThread::currentThread());
    return sidebarModel->insertRow(index, item);
}

bool SideBarWidget::removeItem(SideBarItem *item)
{
    Q_ASSERT(qApp->thread() == QThread::currentThread());
    return sidebarModel->removeRow(item);
}

void SideBarWidget::onItemActived(const QModelIndex &index)
{
    SideBarItem *item = sidebarModel->itemFromIndex(index);
    if (dynamic_cast<SideBarItemSeparator *>(item))
        return;
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    QUrl url { qvariant_cast<QUrl>(item->data(SideBarItem::Roles::ItemUrlRole)) };
    if (!url.isEmpty())
        SideBarEventCaller::sendItemActived(this, url);
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

void SideBarWidget::initDefaultModel()
{
    // create defualt separator line;
    for (const QString &group : currentGroups)
        addItem(SideBarHelper::createSeparatorItem(group));

    // create defualt items
    addItem(SideBarHelper::createDefaultItem("Home", SideBar::DefaultGroup::kCommon));
    addItem(SideBarHelper::createDefaultItem("Desktop", SideBar::DefaultGroup::kCommon));
    addItem(SideBarHelper::createDefaultItem("Videos", SideBar::DefaultGroup::kCommon));
    addItem(SideBarHelper::createDefaultItem("Music", SideBar::DefaultGroup::kCommon));
    addItem(SideBarHelper::createDefaultItem("Pictures", SideBar::DefaultGroup::kCommon));
    addItem(SideBarHelper::createDefaultItem("Documents", SideBar::DefaultGroup::kCommon));
    addItem(SideBarHelper::createDefaultItem("Downloads", SideBar::DefaultGroup::kCommon));

    auto &&infos = SideBarHelper::allCacheInfo();
    for (auto info : infos) {
        addItem(SideBarHelper::createItemByInfo(info));
    }

    // init done, then we should update the separator visible state.
    updateSeparatorVisibleState();
}

void SideBarWidget::initConnect()
{
    // do `cd` work
    connect(sidebarView, &SideBarView::activated,
            this, &SideBarWidget::onItemActived);

    // we need single click also trigger activated()
    connect(sidebarView, &SideBarView::clicked,
            this, &SideBarWidget::onItemActived);

    // context menu
    connect(sidebarView, &SideBarView::customContextMenuRequested,
            this, &SideBarWidget::customContextMenuCall);

    // so no extra separator if a group is empty.
    // since we do this, ensure we do initConnection() after initModelData().
    connect(sidebarModel, &SideBarModel::rowsInserted, this, &SideBarWidget::updateSeparatorVisibleState);
    connect(sidebarModel, &SideBarModel::rowsRemoved, this, &SideBarWidget::updateSeparatorVisibleState);
    connect(sidebarModel, &SideBarModel::rowsMoved, this, &SideBarWidget::updateSeparatorVisibleState);
}

void SideBarWidget::updateSeparatorVisibleState()
{
    QString lastGroupName = "__not_existed_group";
    int lastGroupItemCount = 0;
    int lastSeparatorIndex = -1;

    for (int i = 0; i < sidebarModel->rowCount(); i++) {
        SideBarItem *item = sidebarModel->itemFromIndex(i);
        if (item->group() != lastGroupName) {
            if (dynamic_cast<SideBarItemSeparator *>(item)) {   // Separator
                sidebarView->setRowHidden(i, lastGroupItemCount == 0);
                lastSeparatorIndex = i;
                lastGroupItemCount = 0;
                lastGroupName = item->group();
            }
        } else {
            if (!dynamic_cast<SideBarItemSeparator *>(item))   // SidebarItem
                lastGroupItemCount++;
        }
    }

    // hide the last one if last group is empty
    if (lastGroupItemCount == 0) {
        sidebarView->setRowHidden(lastSeparatorIndex, true);
    }
}
