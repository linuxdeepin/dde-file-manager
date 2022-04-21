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
#include "utils/sidebarmanager.h"
#include "utils/sidebarinfocachemananger.h"

#include "services/filemanager/sidebar/sidebar_defines.h"
#include "dfm-base/utils/systempathutil.h"

#include <QApplication>
#include <QVBoxLayout>
#include <QScrollBar>
#include <QThread>

DPSIDEBAR_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DSB_FM_USE_NAMESPACE

static constexpr char kNotExistedGroup[] { "__not_existed_group" };

SideBarWidget::SideBarWidget(QFrame *parent)
    : AbstractFrame(parent),
      sidebarView(new SideBarView(this)),
      sidebarModel(new SideBarModel(this)),
      currentGroups { SideBar::DefaultGroup::kCommon, SideBar::DefaultGroup::kDevice,
                      SideBar::DefaultGroup::kBookmark, SideBar::DefaultGroup::kNetwork,
                      SideBar::DefaultGroup::kTag, SideBar::DefaultGroup::kOther, kNotExistedGroup }
{
    initializeUi();
    initDefaultModel();
    initConnect();
}

void SideBarWidget::setCurrentUrl(const QUrl &url)
{
    sidebarUrl = url;
    int index = findItem(url);
    if (index != -1) {
        sidebarView->setCurrentIndex(sidebarModel->index(index, 0));
    } else {
        sidebarView->clearSelection();
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

bool SideBarWidget::removeItem(const QUrl &url)
{
    Q_ASSERT(qApp->thread() == QThread::currentThread());
    return sidebarModel->removeRow(url);
}

void SideBarWidget::updateItem(const QUrl &url, const QString &newName, bool editable)
{
    Q_ASSERT(qApp->thread() == QThread::currentThread());
    sidebarModel->updateRow(url, newName, editable);
}

/*!
 * \brief Find the index of the first item match the given \a url
 * \param url
 * \return the index of the item we can found, or -1 if not found.
 */
int SideBarWidget::findItem(const QUrl &url) const
{
    for (int i = 0; i < sidebarModel->rowCount(); i++) {
        SideBarItem *item = sidebarModel->itemFromIndex(i);
        if (!dynamic_cast<SideBarItemSeparator *>(item)) {
            bool foundByCb = item->itemInfo().findMeCb && item->itemInfo().findMeCb(item->url(), url);
            if (foundByCb || (item->url().scheme() == url.scheme() && item->url().path() == url.path()))
                return i;
        }
    }

    return -1;
}

void SideBarWidget::editItem(const QUrl &url)
{
    int pos = findItem(url);
    if (pos < 0)
        return;

    auto idx = sidebarModel->index(pos, 0);
    if (idx.isValid())
        sidebarView->edit(idx);
}

void SideBarWidget::onItemActived(const QModelIndex &index)
{
    SideBarItem *item = sidebarModel->itemFromIndex(index);
    if (dynamic_cast<SideBarItemSeparator *>(item))
        return;

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    QUrl url { qvariant_cast<QUrl>(item->data(SideBarItem::Roles::ItemUrlRole)) };
    SideBarManager::instance()->runCd(item, SideBarHelper::windowId(this));
}

void SideBarWidget::customContextMenuCall(const QPoint &pos)
{
    SideBarItem *item = sidebarView->itemAt(pos);
    if (!item)
        return;

    // 相对坐标转全局坐标
    const QPoint &globalPos = sidebarView->mapToGlobal(pos);

    SideBarManager::instance()->runContextMenu(item, SideBarHelper::windowId(this), globalPos);
}

void SideBarWidget::onItemRenamed(const QModelIndex &index, const QString &newName)
{
    SideBarItem *item = sidebarModel->itemFromIndex(index);
    if (!item)
        return;

    QUrl url { qvariant_cast<QUrl>(item->data(SideBarItem::Roles::ItemUrlRole)) };
    SideBarManager::instance()->runRename(item, SideBarHelper::windowId(this), newName);
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

    // use cahce info
    auto allGroup = SideBarInfoCacheMananger::instance()->groups();
    std::for_each(allGroup.cbegin(), allGroup.cend(), [this](const QString &name) {
        auto list = SideBarInfoCacheMananger::instance()->indexCacheMap(name);
        for (auto &&info : list) {
            SideBarItem *item = SideBarHelper::createItemByInfo(info);
            addItem(item);
        }
    });

    // create defualt items
    static std::once_flag flag;
    std::call_once(flag, [this]() {
        static const QStringList names { "Home", "Desktop", "Videos", "Music", "Pictures", "Documents", "Downloads" };

        for (const QString &name : names) {
            SideBarItem *item = SideBarHelper::createDefaultItem(name, SideBar::DefaultGroup::kCommon);
            addItem(item);
            SideBarInfoCacheMananger::instance()->addItemInfoCache(item->itemInfo());
        }
    });

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

    auto delegate = qobject_cast<SideBarItemDelegate *>(sidebarView->itemDelegate());
    if (delegate)
        connect(delegate, &SideBarItemDelegate::rename, this, &SideBarWidget::onItemRenamed);
    // so no extra separator if a group is empty.
    // since we do this, ensure we do initConnection() after initModelData().
    connect(sidebarModel, &SideBarModel::rowsInserted, this, &SideBarWidget::updateSeparatorVisibleState);
    connect(sidebarModel, &SideBarModel::rowsRemoved, this, &SideBarWidget::updateSeparatorVisibleState);
    connect(sidebarModel, &SideBarModel::rowsMoved, this, &SideBarWidget::updateSeparatorVisibleState);
}

void SideBarWidget::updateSeparatorVisibleState()
{
    QString lastGroupName = kNotExistedGroup;
    int lastGroupItemCount = 0;
    int lastShowSeparatorIndex = -1;

    for (int i = 0; i < sidebarModel->rowCount(); i++) {
        SideBarItem *item = sidebarModel->itemFromIndex(i);
        if (item->group() != lastGroupName) {
            if (dynamic_cast<SideBarItemSeparator *>(item)) {   // Separator
                if (lastGroupItemCount == 0) {
                    sidebarView->setRowHidden(i, true);
                } else {
                    lastShowSeparatorIndex = i;
                    sidebarView->setRowHidden(i, false);
                }
                lastGroupItemCount = 0;
                lastGroupName = item->group();
            }
        } else {
            if (!dynamic_cast<SideBarItemSeparator *>(item))   // SidebarItem
                lastGroupItemCount++;
        }
    }

    // hide the last one
    if (lastShowSeparatorIndex > 0) {
        SideBarItem *item = sidebarModel->itemFromIndex(lastShowSeparatorIndex);
        if (dynamic_cast<SideBarItemSeparator *>(item))
            sidebarView->setRowHidden(lastShowSeparatorIndex, true);
    }
}
