// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sidebarwidget.h"
#include "models/sidebarmodel.h"
#include "views/sidebarview.h"
#include "views/sidebaritem.h"
#include "views/sidebaritemdelegate.h"
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

DPSIDEBAR_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

static constexpr char kNotExistedGroup[] { "__not_existed_group" };
QSharedPointer<SideBarModel> SideBarWidget::kSidebarModelIns { nullptr };

SideBarWidget::SideBarWidget(QFrame *parent)
    : AbstractFrame(parent),
      sidebarView(new SideBarView(this)),
      currentGroups { DefaultGroup::kCommon, DefaultGroup::kDevice,
                      DefaultGroup::kBookmark, DefaultGroup::kNetwork,
                      DefaultGroup::kTag, DefaultGroup::kOther, kNotExistedGroup }
{
    if (!kSidebarModelIns) {
        kSidebarModelIns.reset(new SideBarModel);
        initDefaultModel();
    }
    initializeUi();
    initConnect();
    updateSeparatorVisibleState();
}

void SideBarWidget::setCurrentUrl(const QUrl &url)
{
    sidebarUrl = url;
    int index = findItem(url);
    if (index != -1) {
        sidebarView->setCurrentIndex(kSidebarModelIns->index(index, 0));
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

void SideBarWidget::editItem(const QUrl &url)
{
    int pos = findItem(url);
    if (pos < 0)
        return;

    auto idx = kSidebarModelIns->index(pos, 0);
    if (idx.isValid())
        sidebarView->edit(idx);
}

void SideBarWidget::setItemVisiable(const QUrl &url, bool visible)
{
    Q_ASSERT(qApp->thread() == QThread::currentThread());
    int r = kSidebarModelIns->findRowByUrl(url);
    if (r > 0)
        sidebarView->setRowHidden(r, !visible);
    updateSeparatorVisibleState();
}

void SideBarWidget::updateItemVisiable(const QVariantMap &states)
{
    for (auto iter = states.cbegin(); iter != states.cend(); ++iter) {
        auto urls = SideBarInfoCacheMananger::instance()->findItems(iter.key());
        bool visiable = iter.value().toBool();
        std::for_each(urls.cbegin(), urls.cend(), [visiable, this](const QUrl &url) { setItemVisiable(url, visiable); });
    }
    updateSeparatorVisibleState();
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
    // TODO(zhuangshu): Save some runtime states.(This file is for the listview implementation, maybe no need do that.)
}

void SideBarWidget::onItemActived(const QModelIndex &index)
{
    SideBarItem *item = kSidebarModelIns->itemFromIndex(index);
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
    SideBarItem *item = kSidebarModelIns->itemFromIndex(index);
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

    sidebarView->setModel(kSidebarModelIns.data());
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
            SideBarItem *item = SideBarHelper::createDefaultItem(name, DefaultGroup::kCommon);
            addItem(item);
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
    connect(kSidebarModelIns.data(), &SideBarModel::rowsInserted, this, &SideBarWidget::updateSeparatorVisibleState);
    connect(kSidebarModelIns.data(), &SideBarModel::rowsRemoved, this, &SideBarWidget::updateSeparatorVisibleState);
    connect(kSidebarModelIns.data(), &SideBarModel::rowsMoved, this, &SideBarWidget::updateSeparatorVisibleState);
}

void SideBarWidget::updateSeparatorVisibleState()
{
    QString lastGroupName = kNotExistedGroup;
    int lastGroupItemCount = 0;
    int lastShowSeparatorIndex = -1;
    bool allItemsInvisiable = true;

    for (int i = 0; i < kSidebarModelIns->rowCount(); i++) {
        SideBarItem *item = kSidebarModelIns->itemFromIndex(i);
        bool isSplitter = dynamic_cast<SideBarItemSeparator *>(item);
        if (!isSplitter) {
            if (sidebarView->isRowHidden(i))
                continue;
            else
                allItemsInvisiable = false;
        }

        if (item->group() != lastGroupName) {
            if (isSplitter) {   // Separator
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
        SideBarItem *item = kSidebarModelIns->itemFromIndex(lastShowSeparatorIndex);
        if (dynamic_cast<SideBarItemSeparator *>(item))
            sidebarView->setRowHidden(lastShowSeparatorIndex, true);
    }

    // when no item is visiable in sidebar, do something, such as hide sidebar?
    if (allItemsInvisiable)
        qDebug() << "nothing in sidebar is visiable, maybe hide sidebar?";
}
