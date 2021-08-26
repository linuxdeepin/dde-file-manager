/*
 * Copyright (C) 2019 Deepin Technology Co., Ltd.
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
#include "dfmsidebar.h"

#include "base/dfmapplication.h"
#include "base/dfmsettings.h"
#include "base/singleton.h"
#include "base/define.h"

#include "dfmsidebarview.h"
#include "dfmsidebarmanager.h"

#include "dfmsidebarmodel.h"
#include "dfmsidebaritemdelegate.h"
#include "dfmsidebaritem.h"

//#include "localfile/dfileservices.h"
//#include "localfile/filesignalmanager.h"

#include <DApplicationHelper>
#include <QScrollBar>
#include <QVBoxLayout>
#include <QDebug>

#include <QMenu>
#include <QtConcurrent>
#include <QApplication>

#include <algorithm>

#define SIDEBAR_ITEMORDER_KEY "SideBar/ItemOrder"

DFMSideBar::DFMSideBar(QWidget *parent)
    : QWidget(parent),
      m_sidebarView(new DFMSideBarView(this)),
      m_sidebarModel(new DFMSideBarModel(this))
{

    // init view.
    m_sidebarView->setModel(m_sidebarModel);
    m_sidebarView->setItemDelegate(new DFMSideBarItemDelegate(m_sidebarView));
    m_sidebarView->setViewportMargins(10, 0, m_sidebarView->verticalScrollBar()->sizeHint().width(), 0);
    m_sidebarView->setContextMenuPolicy(Qt::CustomContextMenu);
    m_sidebarView->setFrameShape(QFrame::Shape::NoFrame);
    m_sidebarView->setAutoFillBackground(true);

    initUI();
//    initModelData();
    initConnection();
}

DFMSideBar::~DFMSideBar()
{
#ifdef ENABLE_ASYNCINIT
    m_initDevThread.first = true;
    m_initDevThread.second.waitForFinished();
#endif
}

QAbstractItemView *DFMSideBar::view()
{
    return m_sidebarView;
}

QRect DFMSideBar::groupGeometry(const QString &groupName)
{
    return  m_sidebarView->visualRect(groupModelIndex(groupName));
}

void DFMSideBar::scrollToGroup(const QString &groupName)
{
    m_sidebarView->scrollTo(groupModelIndex(groupName));
}

void DFMSideBar::setCurrentUrl(const QUrl &url)
{
    int index = findItem(url, true);
    if (index != -1) {
        m_sidebarView->setCurrentIndex(m_sidebarModel->index(index, 0));
    } else {
        m_sidebarView->clearSelection();
    }
}

int DFMSideBar::addItem(DFMSideBarItem *item, const QString &group)
{
    if (!item) {
        return 0;
    }
    int lastAtGroup = findLastItem(group, false);
    lastAtGroup++; // append after the last item
    this->insertItem(lastAtGroup, item, group);

    return lastAtGroup;
}

bool DFMSideBar::removeItem(const QUrl &url, const QString &group)
{
    int index = findItem(url, group);
    bool succ = false;
    if (index >= 0) {
        succ = m_sidebarModel->removeRow(index);
    }

    return succ;
}

int DFMSideBar::findItem(const DFMSideBarItem *item) const
{
    return m_sidebarModel->indexFromItem(item).row();
}

int DFMSideBar::findItem(const QUrl &url, const QString &group) const
{
    for (int i = 0; i < m_sidebarModel->rowCount(); i++) {
        DFMSideBarItem *item = m_sidebarModel->itemFromIndex(i);
        if (!dynamic_cast<DFMSideBarItemSeparator*>(item) && item->group() == group) {
            if (item->url() == url) {
                return i;
            }
        }
    }

    return -1;
}

/*!
 * \brief Find the index of the first item match the given \a url
 *
 * \return the index of the item we can found, or -1 if not found.
 */
int DFMSideBar::findItem(const QUrl &url, bool fuzzy/* = false*/) const
{
    for (int i = 0; i < m_sidebarModel->rowCount(); i++) {
        DFMSideBarItem *item = m_sidebarModel->itemFromIndex(i);
        if (!dynamic_cast<DFMSideBarItemSeparator*>(item)) {
            if (item->url() == url)
                return i;

            if (!fuzzy)
                continue;

            QUrl itemUrl = item->url();
        }
    }

    return -1;
}

int DFMSideBar::findItem(std::function<bool (const DFMSideBarItem *)> cb) const
{
    for (int i = 0; i < m_sidebarModel->rowCount(); i++) {
        DFMSideBarItem *item = m_sidebarModel->itemFromIndex(i);
        if (cb(item)) {
            return i;
        }
    }

    return -1;
}

int DFMSideBar::findLastItem(const QString &group, bool sidebarItemOnly) const
{
    int index = -1;
    for (int i = 0; i < m_sidebarModel->rowCount(); i++) {
        DFMSideBarItem *item = m_sidebarModel->itemFromIndex(i);
        if (item->group() == group && (dynamic_cast<DFMSideBarItem*>(item) || !sidebarItemOnly)) {
            index = i;
        } else if (item->group() != group && index != -1) {
            // already found the group and already leaved the group
            break;
        }
    }

    return index;
}

void DFMSideBar::openItemEditor(int index) const
{
    m_sidebarView->edit(m_sidebarModel->index(index, 0));
}

QSet<QString> DFMSideBar::disableUrlSchemes() const
{
    return m_disableUrlSchemes;
}

void DFMSideBar::setContextMenuEnabled(bool enabled)
{
    m_contextMenuEnabled = enabled;
}

void DFMSideBar::setDisableUrlSchemes(const QSet<QString> &schemes)
{
    m_disableUrlSchemes += schemes;
    for (QString scheme : m_disableUrlSchemes) {
        Q_FOREVER {
            int index = findItem([&](const DFMSideBarItem * item) -> bool {
                return item->url().scheme() == scheme;
            });

            if (index >= 0) {
                m_sidebarModel->removeRow(index);
            } else {
                break;
            }
        }
    }

    Q_EMIT disableUrlSchemesChanged();
}

QList<QUrl> DFMSideBar::savedItemOrder(const QString &groupName) const
{
    QList<QUrl> list;

    QStringList savedList = DFMApplication::genericSetting()->value(SIDEBAR_ITEMORDER_KEY, groupName).toStringList();
    for (const QString &item : savedList) {
        list << QUrl(item);
    }

    return list;
}

void DFMSideBar::saveItemOrder(const QString &groupName) const
{
    QVariantList list;

    for (int i = 0; i < m_sidebarModel->rowCount(); i++) {
        DFMSideBarItem *item = m_sidebarModel->itemFromIndex(m_sidebarModel->index(i, 0));
        if (!dynamic_cast<DFMSideBarItemSeparator*>(item)
                && item->group() == groupName) {
            list << QVariant(item->url());
        }
    }

    DFMApplication::genericSetting()->setValue(SIDEBAR_ITEMORDER_KEY, groupName, list);
}

QString DFMSideBar::groupName(DFMSideBar::GroupName group)
{
    auto metaEnum = QMetaEnum::fromType<DFMSideBar::GroupName>();
    return metaEnum.valueToKey(group);
}

DFMSideBar::GroupName DFMSideBar::groupFromName(const QString &name)
{
    QMetaEnum groupEnum = QMetaEnum::fromType<DFMSideBar::GroupName>();

    bool result = false;

    DFMSideBar::GroupName enumKey = static_cast<DFMSideBar::GroupName>
            (groupEnum.keyToValue(name.toLatin1().data(), &result));

    if (!result) return Unknown;

    return enumKey;
}

void DFMSideBar::onItemActivated(const QModelIndex &index)
{
    DFMSideBarItem *item = m_sidebarModel->itemFromIndex(index);
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    QUrl url = qvariant_cast<QUrl>(item->data(DFMSideBarItem::Roles::ItemUrlRole));
    if (!url.isEmpty())
        Q_EMIT activatedItemUrl(url);
}

void DFMSideBar::onContextMenuRequested(const QPoint &pos)
{
    // block signals to avoid function reentrant.
    const QSignalBlocker blocker(this);

    if (!m_contextMenuEnabled) return;


    QModelIndex modelIndex = m_sidebarView->indexAt(pos);
    if (!modelIndex.isValid()) {
        return;
    }

    DFMSideBarItem *item = m_sidebarModel->itemFromIndex(modelIndex);
    if (!item || dynamic_cast<DFMSideBarItemSeparator*>(item)) {
        return ; // separator should not show menu
    }
//    DFileService::instance()->setCursorBusyState(false);
    return;
}

void DFMSideBar::onRename(const QModelIndex &index, const QString &newName) const
{
    DFMSideBarItem *item = m_sidebarModel->itemFromIndex(index);
    if (!newName.isEmpty() && item->text() != newName) {
        item->setData(newName);
    }
}

void DFMSideBar::initUI()
{
    // init layout.
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_sidebarView);

    layout->setMargin(0);
    layout->setSpacing(0);

    this->setMaximumWidth(200);
    this->setFocusProxy(m_sidebarView);

    applySidebarColor();
}

void DFMSideBar::initModelData()
{
    qRegisterMetaTypeStreamOperators<QUrl>("QUrl");

    static QList<DFMSideBar::GroupName> groups {
        GroupName::StandardPath
    };

    Q_FOREACH (const DFMSideBar::GroupName &groupType, groups) {
        auto separator = new DFMSideBarItemSeparator();
        separator->setGroup(groupName(groupType));
        m_sidebarModel->appendRow(separator);
        addGroupItems(groupType);
    }

    updateSeparatorVisibleState();
}

void DFMSideBar::initConnection()
{
    // drag to delete bookmark or tag
    connect(m_sidebarView, &DFMSideBarView::requestRemoveItem, this, [this]() {
        DFMSideBarItem *item = m_sidebarModel->itemFromIndex(m_sidebarView->currentIndex());
        if (item && item->flags().testFlag(Qt::ItemIsEnabled) && item->flags().testFlag(Qt::ItemIsDragEnabled)) {
//            DFileService::instance()->deleteFiles(nullptr, QList<QUrl>{item->url()}, false);
        }
    });

    //resend to extensions
    QObject::connect(m_sidebarView, &QListView::activated, this, &DFMSideBar::activateditemIndex);

    // we need single click also trigger activated()
    connect(m_sidebarView, &QListView::clicked, this, &DFMSideBar::onItemActivated);

    // context menu
    connect(m_sidebarView, &QListView::customContextMenuRequested, this, &DFMSideBar::onContextMenuRequested);

    // so no extra separator if a group is empty.
    // since we do this, ensure we do initConnection() after initModelData().
    connect(m_sidebarModel, &QStandardItemModel::rowsInserted, this, &DFMSideBar::updateSeparatorVisibleState);
    connect(m_sidebarModel, &QStandardItemModel::rowsRemoved, this, &DFMSideBar::updateSeparatorVisibleState);
    connect(m_sidebarModel, &QStandardItemModel::rowsMoved, this, &DFMSideBar::updateSeparatorVisibleState);
    // drag to move item will Q_EMIT rowsInserted and rowsMoved..
    connect(m_sidebarModel, &QStandardItemModel::rowsRemoved, this,
            [this](const QModelIndex & parent, int first, int last) {
        Q_UNUSED(parent);
        Q_UNUSED(last);
        DFMSideBarItem *item = m_sidebarModel->itemFromIndex(first);
        if (!item) {
            item = m_sidebarModel->itemFromIndex(first - 1);
        }

        // only bookmark and tag item are DragEnabled
        if (item && item->flags().testFlag(Qt::ItemIsEnabled) && item->flags().testFlag(Qt::ItemIsDragEnabled)) {
            saveItemOrder(item->group());
        }
    });
    DFMSideBarItemDelegate *idelegate = dynamic_cast<DFMSideBarItemDelegate *>(m_sidebarView->itemDelegate());
    if (idelegate) {
        connect(idelegate, &DFMSideBarItemDelegate::rename, this, &DFMSideBar::onRename);
    }

    //    connect(fileSignalManager, &FileSignalManager::requestRename, this, [this](const DFMUrlBaseEvent & event) {
    //        if (event.sender() == this) {
    //            this->openItemEditor(this->findItem(event.url()));
    //        }
    //    });
}

void DFMSideBar::applySidebarColor()
{
    m_sidebarView->setBackgroundType(DStyledItemDelegate::BackgroundType(DStyledItemDelegate::RoundedBackground | DStyledItemDelegate::NoNormalState));
    m_sidebarView->setItemSpacing(0);
}

void DFMSideBar::updateSeparatorVisibleState()
{
    for (int i = 0; i < m_sidebarModel->rowCount(); i++) {
        DFMSideBarItem *item = m_sidebarModel->itemFromIndex(i);
        if (dynamic_cast<DFMSideBarItemSeparator*>(item)) {
            int currSeparatorRow = m_sidebarModel->indexFromItem(item).row();
            if (0 == currSeparatorRow) //分隔符首行屏蔽
                m_sidebarView->setRowHidden(0,true);
            if (0 < currSeparatorRow) { //分隔符非首行
                auto perItem = m_sidebarModel->item((currSeparatorRow - 1));
                bool isSeparator = dynamic_cast<DFMSideBarItemSeparator*>(perItem);
                if (isSeparator) { //前节点是Separator 隐藏
                    m_sidebarView->setRowHidden(currSeparatorRow,true);
                } else {
                    auto perGroup = perItem->data(DFMSideBarItem::Roles::ItemGroupRole);
                    auto currGroup = item->data(DFMSideBarItem::Roles::ItemGroupRole);
                    if (perGroup == currGroup)//同分组分割隐藏
                        m_sidebarView->setRowHidden(currSeparatorRow,true);
                }
            }
        }
    }
}

void DFMSideBar::addGroupItems(DFMSideBar::GroupName groupType)
{
//    const QString &groupNameStr = groupName(groupType);
//    switch (groupType) {
//    case GroupName::StandardPath:
//        appendItem(DFMSideBarDefaultItemHandler::createItem("Home"), groupNameStr);
//        appendItem(DFMSideBarDefaultItemHandler::createItem("Desktop"), groupNameStr);
//        appendItem(DFMSideBarDefaultItemHandler::createItem("Videos"), groupNameStr);
//        appendItem(DFMSideBarDefaultItemHandler::createItem("Music"), groupNameStr);
//        appendItem(DFMSideBarDefaultItemHandler::createItem("Pictures"), groupNameStr);
//        appendItem(DFMSideBarDefaultItemHandler::createItem("Documents"), groupNameStr);
//        appendItem(DFMSideBarDefaultItemHandler::createItem("Downloads"), groupNameStr);
//        break;
//    default:
//        break;
//    }
}

void DFMSideBar::insertItem(int index, DFMSideBarItem *item, const QString &groupName)
{
    if (!item) {
        return;
    }
    item->setGroup(groupName);
    m_sidebarModel->insertRow(index, item);
}

/*!
 * \brief append an \a item to the sidebar item model, with the given \a groupName
 *
 * Warning! Item is directly append to the model, will NOT try to find the group
 * location by the given group name. For that (find group location and append item)
 * purpose, use addItem() instead.
 */
void DFMSideBar::appendItem(DFMSideBarItem *item, const QString &groupName)
{
    item->setGroup(groupName);
    m_sidebarModel->appendRow(item);
}

void DFMSideBar::appendItemWithOrder(QList<DFMSideBarItem *> &list, const QList<QUrl> &order, const QString &groupName)
{
    QList<QUrl> urlList;

    for (const DFMSideBarItem *item : list) {
        urlList << item->url();
    }

    for (const QUrl &url : order) {
        int idx = urlList.indexOf(url);
        if (idx >= 0) {
            urlList.removeAt(idx);
            this->appendItem(list.takeAt(idx), groupName);
        }
    }

    for (DFMSideBarItem *item : list) {
        this->appendItem(item, groupName);
    }
}

/*!
 * \brief Find the model-index of the groupNname
 *
 * \return the the model-index of the groupNname we can found, or invalid-model-index if not found.
 */
QModelIndex DFMSideBar::groupModelIndex(const QString &groupName)
{
    return m_sidebarModel->index(findLastItem(groupName), 0, m_sidebarView->rootIndex());
}

void DFMSideBar::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::PaletteChange) {
        applySidebarColor();
    }

    return QWidget::changeEvent(event);
}


