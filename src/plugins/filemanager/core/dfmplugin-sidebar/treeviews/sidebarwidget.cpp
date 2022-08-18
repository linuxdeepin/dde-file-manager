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
#include "sidebarmodel.h"
#include "sidebarview.h"
#include "sidebaritem.h"
#include "sidebaritemdelegate.h"
#include "events/sidebareventcaller.h"
#include "utils/sidebarhelper.h"
#include "utils/sidebarmanager.h"
#include "utils/sidebarinfocachemananger.h"

#include "dfm-base/widgets/dfmwindow/filemanagerwindowsmanager.h"
#include "dfm-base/utils/systempathutil.h"

#include <QApplication>
#include <QVBoxLayout>
#include <QScrollBar>
#include <QThread>
#include <QDebug>
#include <QCloseEvent>
#include <QTimer>

DPSIDEBAR_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

QSharedPointer<SideBarModel> SideBarWidget::kSidebarModelIns { nullptr };

SideBarWidget::SideBarWidget(QFrame *parent)
    : AbstractFrame(parent),
      sidebarView(new SideBarView(this))
{
    if (!kSidebarModelIns) {
        kSidebarModelIns.reset(new SideBarModel);
        initDefaultModel();
    }
    initializeUi();
    initConnect();
    sidebarView->updateSeparatorVisibleState();
}

void SideBarWidget::setCurrentUrl(const QUrl &url)
{
    sidebarView->setCurrentUrl(url);
    const QModelIndex &index = findItemIndex(url);
    if (!index.isValid() || index.row() < 0 || index.column() < 0) {
        sidebarView->clearSelection();
        return;
    }

    SideBarItem *currentItem = kSidebarModelIns->itemFromIndex(index);
    if (currentItem && currentItem->parent()) {
        SideBarItemSeparator *groupItem = dynamic_cast<SideBarItemSeparator *>(currentItem->parent());
        //If the current item's group is not expanded, do not set current index, otherwise
        //the unexpanded group would be expaned again.
        if (groupItem && !groupItem->isExpanded())
            return;
    }

    sidebarView->setCurrentIndex(index);
}

QUrl SideBarWidget::currentUrl() const
{
    return sidebarView->currentUrl();
}

void SideBarWidget::changeEvent(QEvent *event)
{
    return QWidget::changeEvent(event);
}

QAbstractItemView *SideBarWidget::view()
{
    return sidebarView;
}

int SideBarWidget::addItem(SideBarItem *item)
{
    qInfo() << "item = " << item->url();
    Q_ASSERT(qApp->thread() == QThread::currentThread());
    // TODO(zhangs): custom group
    int r { kSidebarModelIns->appendRow(item) };
    bool hideAddedItem = r >= 0 && SideBarInfoCacheMananger::instance()->containsHiddenUrl(item->url());
    if (!SideBarHelper::hiddenRules().value(item->itemInfo().visiableControlKey, true).toBool())
        hideAddedItem = true;

    if (hideAddedItem)
        setItemVisiable(item->url(), false);
    return r;
}

bool SideBarWidget::insertItem(const int index, SideBarItem *item)
{
    qInfo() << "item = " << item->url();
    Q_ASSERT(qApp->thread() == QThread::currentThread());

    bool r { kSidebarModelIns->insertRow(index, item) };
    bool hideInsertedItem = r && SideBarInfoCacheMananger::instance()->containsHiddenUrl(item->url());
    if (!SideBarHelper::hiddenRules().value(item->itemInfo().visiableControlKey, true).toBool())
        hideInsertedItem = true;

    if (hideInsertedItem)
        setItemVisiable(item->url(), false);
    return r;
}

bool SideBarWidget::removeItem(SideBarItem *item)
{
    Q_ASSERT(qApp->thread() == QThread::currentThread());
    return kSidebarModelIns->removeRow(item);
}

bool SideBarWidget::removeItem(const QUrl &url)
{
    Q_ASSERT(qApp->thread() == QThread::currentThread());
    return kSidebarModelIns->removeRow(url);
}

void SideBarWidget::updateItem(const QUrl &url, const ItemInfo &newInfo)
{
    Q_ASSERT(qApp->thread() == QThread::currentThread());
    kSidebarModelIns->updateRow(url, newInfo);
}

/*!
 * \brief Find the index of the first item match the given \a url
 * \param url
 * \return the index of the item we can found, or -1 if not found.
 */
int SideBarWidget::findItem(const QUrl &url) const
{
    // TODO(zhuangshu): In sidebar tree-model mode, this function is deprecated and be instead of findItemIndex()
    for (int i = 0; i < kSidebarModelIns->rowCount(); i++) {
        SideBarItem *item = kSidebarModelIns->itemFromIndex(i);
        if (!dynamic_cast<SideBarItemSeparator *>(item)) {
            bool foundByCb = item->itemInfo().findMeCb && item->itemInfo().findMeCb(item->url(), url);
            if (foundByCb || (item->url().scheme() == url.scheme() && item->url().path() == url.path()))
                return i;
        }
    }

    return -1;
}
//zhuangshu: Currently, function findItemIndex can only support to find out the sub item of group,
//so it can not find out the top item (group item),
//but do not effect our function.
QModelIndex SideBarWidget::findItemIndex(const QUrl &url) const
{
    int count = kSidebarModelIns->rowCount();
    for (int i = 0; i < count; i++) {
        SideBarItem *topItem = kSidebarModelIns->itemFromIndex(i);
        SideBarItemSeparator *groupItem = dynamic_cast<SideBarItemSeparator *>(topItem);
        if (groupItem) {
            int childCount = groupItem->rowCount();
            for (int j = 0; j < childCount; j++) {
                QStandardItem *childItem = groupItem->child(j);
                SideBarItem *item = dynamic_cast<SideBarItem *>(childItem);
                if (!item)
                    continue;
                bool foundByCb = item->itemInfo().findMeCb && item->itemInfo().findMeCb(item->url(), url);
                if (foundByCb || (item->url().scheme() == url.scheme() && item->url().path() == url.path()))
                    return item->index();
            }
        }
    }

    return QModelIndex();
}

void SideBarWidget::editItem(const QUrl &url)
{
    QModelIndex ret = findItemIndex(url);
    int pos = ret.row();
    if (pos < 0)
        return;

    auto idx = kSidebarModelIns->index(pos, 0, ret.parent());
    if (idx.isValid())
        sidebarView->edit(idx);
}

void SideBarWidget::setItemVisiable(const QUrl &url, bool visible)
{
    qInfo() << "url = " << url << ",visible = " << visible;

    Q_ASSERT(qApp->thread() == QThread::currentThread());
    //find out the item index by url
    const QModelIndex index = this->findItemIndex(url);   //ps: currently,findItemIndex can only find the sub item
    if (!index.isValid()) {
        qInfo() << "index is invalid";
        return;
    }
    QStandardItem *item = qobject_cast<const SideBarModel *>(index.model())->itemFromIndex(index);
    if (item && item->parent())
        sidebarView->setRowHidden(item->row(), item->parent()->index(), !visible);

    sidebarView->updateSeparatorVisibleState();
}

void SideBarWidget::updateItemVisiable(const QVariantMap &states)
{
    for (auto iter = states.cbegin(); iter != states.cend(); ++iter) {
        auto urls = SideBarInfoCacheMananger::instance()->findItems(iter.key());
        bool visiable = iter.value().toBool();
        std::for_each(urls.cbegin(), urls.cend(), [visiable, this](const QUrl &url) { setItemVisiable(url, visiable); });
    }

    sidebarView->updateSeparatorVisibleState();
}

QList<QUrl> SideBarWidget::findItems(const QString &group) const
{
    QList<QUrl> ret;
    bool groupTraversed { false };
    for (int r = 0; r < kSidebarModelIns->rowCount(); r++) {
        auto item = kSidebarModelIns->itemFromIndex(kSidebarModelIns->index(r, 0));
        if (item && item->group() == group) {
            auto u = item->url();
            if (u.isValid())
                ret << u;
            groupTraversed = true;
        } else {
            if (groupTraversed)
                break;
        }
    }
    return ret;
}

void SideBarWidget::updateSelection()
{
    // after sort the hightlight may lose, re-select the highlight item.
    quint64 winId = SideBarHelper::windowId(this);
    auto window = FMWindowsIns.findWindowById(winId);
    if (window)
        setCurrentUrl(window->currentUrl());
}

void SideBarWidget::saveStateWhenClose()
{
    sidebarView->saveStateWhenClose();
}

void SideBarWidget::onItemActived(const QModelIndex &index)
{
    SideBarItem *item = kSidebarModelIns->itemFromIndex(index);
    if (dynamic_cast<SideBarItemSeparator *>(item))
        return;

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    QUrl url { qvariant_cast<QUrl>(item->data(SideBarItem::Roles::kItemUrlRole)) };
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
    SideBarItem *item = kSidebarModelIns->itemFromIndex(index);
    if (!item)
        return;

    QUrl url { qvariant_cast<QUrl>(item->data(SideBarItem::Roles::kItemUrlRole)) };
    SideBarManager::instance()->runRename(item, SideBarHelper::windowId(this), newName);
}

void SideBarWidget::initializeUi()
{
    QVBoxLayout *vlayout = new QVBoxLayout(this);
    vlayout->addWidget(sidebarView);
    vlayout->setMargin(0);
    vlayout->setSpacing(0);

    sidebarView->setModel(kSidebarModelIns.data());
    sidebarView->setItemDelegate(new SideBarItemDelegate(sidebarView));
    sidebarView->setContextMenuPolicy(Qt::CustomContextMenu);
    sidebarView->setFrameShape(QFrame::Shape::NoFrame);
    sidebarView->setAutoFillBackground(true);

    setMaximumWidth(200);
    setFocusProxy(sidebarView);
}

void SideBarWidget::initDefaultModel()
{
    currentGroups << DefaultGroup::kCommon
                  << DefaultGroup::kDevice
                  << DefaultGroup::kBookmark
                  << DefaultGroup::kNetwork
                  << DefaultGroup::kTag
                  << DefaultGroup::kOther
                  << DefaultGroup::kNotExistedGroup;

    groupDisplayName.insert(DefaultGroup::kCommon, tr("Quick access"));
    groupDisplayName.insert(DefaultGroup::kDevice, tr("Partitions"));
    groupDisplayName.insert(DefaultGroup::kNetwork, tr("Network"));
    groupDisplayName.insert(DefaultGroup::kTag, tr("Tag"));
    groupDisplayName.insert(DefaultGroup::kBookmark, tr("Bookmark"));
    groupDisplayName.insert(DefaultGroup::kOther, tr("Other"));
    groupDisplayName.insert(DefaultGroup::kNotExistedGroup, tr("Unkown Group"));

    // create defualt separator line;
    QMap<QString, SideBarItem *> temGroupItem;
    for (const QString &group : currentGroups) {
        auto item = SideBarHelper::createSeparatorItem(group);
        item->setData(groupDisplayName.value(group), Qt::DisplayRole);
        addItem(item);
        temGroupItem.insert(group, item);
    }

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
            SideBarItem *item = SideBarHelper::createDefaultItem(name, DefaultGroup::kCommon);
            addItem(item);
            SideBarInfoCacheMananger::instance()->addItemInfoCache(item->itemInfo());
        }
    });

    // init done, then we should update the separator visible state.
    sidebarView->updateSeparatorVisibleState();
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
    if (delegate) {
        connect(delegate, &SideBarItemDelegate::rename, this, &SideBarWidget::onItemRenamed);
        connect(delegate, &SideBarItemDelegate::changeExpandState, sidebarView, &SideBarView::onChangeExpandState);
    }

    // so no extra separator if a group is empty.
    // since we do this, ensure we do initConnection() after initModelData().
    connect(kSidebarModelIns.data(), &SideBarModel::rowsInserted, sidebarView, &SideBarView::updateSeparatorVisibleState);
    connect(kSidebarModelIns.data(), &SideBarModel::rowsRemoved, sidebarView, &SideBarView::updateSeparatorVisibleState);
    connect(kSidebarModelIns.data(), &SideBarModel::rowsMoved, sidebarView, &SideBarView::updateSeparatorVisibleState);
}
