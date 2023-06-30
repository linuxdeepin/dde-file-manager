// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sidebarwidget.h"
#include "sidebarmodel.h"
#include "sidebarview.h"
#include "sidebaritem.h"
#include "sidebaritemdelegate.h"
#include "events/sidebareventcaller.h"
#include "utils/sidebarhelper.h"
#include "utils/sidebarmanager.h"
#include "utils/sidebarinfocachemananger.h"

#include <dfm-framework/event/event.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-base/utils/systempathutil.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/utils/networkutils.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/settings.h>
#include <dfm-base/utils/dialogmanager.h>

#include <QApplication>
#include <QVBoxLayout>
#include <QScrollBar>
#include <QThread>
#include <QDebug>
#include <QCloseEvent>
#include <QTimer>

using namespace dfmplugin_sidebar;
DFMBASE_USE_NAMESPACE

QSharedPointer<SideBarModel> SideBarWidget::kSidebarModelIns { nullptr };

SideBarWidget::SideBarWidget(QFrame *parent)
    : AbstractFrame(parent),
      sidebarView(new SideBarView(this))
{
#ifdef ENABLE_TESTING
    dpfSlotChannel->push("dfmplugin_utils", "slot_Accessible_SetAccessibleName",
                         qobject_cast<QWidget *>(sidebarView), AcName::kAcDmSideBarView);

#endif
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
    Q_ASSERT(qApp->thread() == QThread::currentThread());

    bool r { kSidebarModelIns->insertRow(index, item) };
    bool hideInsertedItem = r && SideBarInfoCacheMananger::instance()->containsHiddenUrl(item->url());
    if (!SideBarHelper::hiddenRules().value(item->itemInfo().visiableControlKey, true).toBool())
        hideInsertedItem = true;

    if (hideInsertedItem)
        setItemVisiable(item->url(), false);

    return r;
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
// zhuangshu: Currently, function findItemIndex can only support to find out the sub item of group,
// so it can not find out the top item (group item),
// but do not effect our function.
QModelIndex SideBarWidget::findItemIndex(const QUrl &url) const
{
    return sidebarView->findItemIndex(url);
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
    // find out the item index by url
    const QModelIndex index = this->findItemIndex(url);   // ps: currently,findItemIndex can only find the sub item
    if (!index.isValid()) {
        qInfo() << "index is invalid";
        return;
    }
    QStandardItem *item = qobject_cast<const SideBarModel *>(index.model())->itemFromIndex(index);
    if (item && item->parent()) {
        sidebarView->setRowHidden(item->row(), item->parent()->index(), !visible);
    }

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
    for (int r = 0; r < kSidebarModelIns->rowCount(); r++) {
        auto item = kSidebarModelIns->itemFromIndex(kSidebarModelIns->index(r, 0));
        if (!(item && item->group() == group))
            continue;
        for (int i = 0; i < item->rowCount(); i++) {
            QStandardItem *subItem = item->child(i);
            if (!subItem)
                continue;
            auto u = subItem->index().data(SideBarItem::kItemUrlRole).toUrl();
            if (u.isValid())
                ret << u;
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
    if (!item || dynamic_cast<SideBarItemSeparator *>(item))
        return;

    DViewItemActionList list = item->actionList(Qt::RightEdge);
    if (list.count() > 0 && !list.first()->isEnabled()) {
        list.first()->setDisabled(false);
        setCurrentUrl(list.first()->property("currentItem").toUrl());   // for keeping the selected item.
        return;
    }

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    QUrl url { qvariant_cast<QUrl>(item->data(SideBarItem::Roles::kItemUrlRole)) };
    if (NetworkUtils::instance()->checkFtpOrSmbBusy(url)) {
        DialogManager::instance()->showUnableToVistDir(url.path());
        QApplication::restoreOverrideCursor();
        auto preIndex = sidebarView->previousIndex();
        if (!preIndex.isValid()) {
            sidebarView->setPreviousIndex(preIndex);
            return;
        }
        SideBarItem *preItem = kSidebarModelIns->itemFromIndex(preIndex);
        if (!preItem || dynamic_cast<SideBarItemSeparator *>(preItem))
            return;
        setCurrentUrl(qvariant_cast<QUrl>(preItem->data(SideBarItem::Roles::kItemUrlRole)));
        sidebarView->setPreviousIndex(preIndex);
        return;
    }

    QApplication::restoreOverrideCursor();
    SideBarManager::instance()->runCd(item, SideBarHelper::windowId(this));
    sidebarView->update(sidebarView->previousIndex());
    sidebarView->update(sidebarView->currentIndex());
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

    QWidget *spacer = new QWidget(this);
    spacer->setAutoFillBackground(true);
    spacer->setFixedHeight(10);
    spacer->setBackgroundRole(QPalette::Base);

    vlayout->addWidget(sidebarView);
    vlayout->setMargin(0);
    vlayout->setSpacing(0);
    vlayout->addWidget(spacer);

    sidebarView->setModel(kSidebarModelIns.data());
    sidebarView->setItemDelegate(new SideBarItemDelegate(sidebarView));
    sidebarView->setContextMenuPolicy(Qt::CustomContextMenu);
    sidebarView->setFrameShape(QFrame::Shape::NoFrame);
    sidebarView->setAutoFillBackground(true);
    setMaximumWidth(200);   // Set the sidebar's maximum width

    const QVariantMap &state = Application::appObtuselySetting()->value("WindowManager", "SplitterState").toMap();
    int splitState = state.value("sidebar", 200).toInt();
    QSize size = this->size();
    size.setWidth(splitState);
    resize(size);

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
    groupDisplayName.insert(DefaultGroup::kNotExistedGroup, tr("Unknown Group"));

    // create defualt separator item.
    for (const QString &group : currentGroups) {
        auto item = SideBarHelper::createSeparatorItem(group);
        item->setData(groupDisplayName.value(group), Qt::DisplayRole);
        addItem(item);
    }

    // use cache info to create items of groups.
    auto allGroup = SideBarInfoCacheMananger::instance()->groups();
    std::for_each(allGroup.cbegin(), allGroup.cend(), [this](const QString &name) {
        auto list = SideBarInfoCacheMananger::instance()->indexCacheMap(name);
        for (auto &&info : list) {
            SideBarItem *item = SideBarHelper::createItemByInfo(info);
            addItem(item);
        }
    });

    // The following code is moved to bookmark plugin.
    /*
     //create defualt items
        static std::once_flag flag;
        std::call_once(flag, [this]() {
            static const QStringList names { "Home", "Desktop", "Videos", "Music", "Pictures", "Documents", "Downloads" };

            for (const QString &name : names) {
                SideBarItem *item = SideBarHelper::createDefaultItem(name, DefaultGroup::kCommon);
                addItem(item);
                SideBarInfoCacheMananger::instance()->addItemInfoCache(item->itemInfo());
            }
        });
    */
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
