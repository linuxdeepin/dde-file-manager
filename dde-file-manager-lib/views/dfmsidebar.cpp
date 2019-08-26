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

#include "dfmapplication.h"
#include "dfmsettings.h"
#include "dabstractfilewatcher.h"
#include "dfilemanagerwindow.h"
#include "dfileservices.h"
#include "singleton.h"

#include "dfmsidebarmanager.h"
#include "interfaces/dfmsidebariteminterface.h"
#include "views/dfmsidebarview.h"
#include "models/dfmsidebarmodel.h"
#include "dfmsidebaritemdelegate.h"
#include "dfmsidebaritem.h"
#include "controllers/dfmsidebardefaultitemhandler.h"
#include "controllers/dfmsidebarbookmarkitemhandler.h"
#include "controllers/dfmsidebardeviceitemhandler.h"
#include "controllers/dfmsidebartagitemhandler.h"
#include "controllers/dfmsidebaropticalitemhandler.h"

#include <QVBoxLayout>
#include <QDebug>

#include <ddiskmanager.h>
#include <ddiskdevice.h>
#include <dblockdevice.h>
#include <QMenu>

#define SIDEBAR_ITEMORDER_KEY "SideBar/ItemOrder"

DFM_BEGIN_NAMESPACE

DFMSideBar::DFMSideBar(QWidget *parent)
    : QWidget(parent),
      m_sidebarView(new DFMSideBarView(this)),
      m_sidebarModel(new DFMSideBarModel(this))
{
    // init view.
    m_sidebarView->setModel(m_sidebarModel);
    m_sidebarView->setItemDelegate(new DFMSideBarItemDelegate(m_sidebarView));
    m_sidebarView->setContextMenuPolicy(Qt::CustomContextMenu);

    initUI();
    initModelData();
    initConnection();
}

void DFMSideBar::setCurrentUrl(const DUrl &url)
{
    Q_UNUSED(url);
}

int DFMSideBar::addItem(DFMSideBarItem *item, const QString &group)
{
    int lastAtGroup = findLastItem(group);
    lastAtGroup++; // append after the last item
    this->insertItem(lastAtGroup, item, group);

    return lastAtGroup;
}

bool DFMSideBar::removeItem(const DUrl &url, const QString &group)
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

int DFMSideBar::findItem(const DUrl &url, const QString &group) const
{
    for (int i = 0; i < m_sidebarModel->rowCount(); i++) {
        DFMSideBarItem * item = m_sidebarModel->itemFromIndex(i);
        if (item->itemType() == DFMSideBarItem::SidebarItem && item->groupName() == group) {
            if (item->url() == url) {
                return i;
            }
        }
    }

    return -1;
}

int DFMSideBar::findLastItem(const QString &group) const
{
    int index = -1;
    for (int i = 0; i < m_sidebarModel->rowCount(); i++) {
        DFMSideBarItem * item = m_sidebarModel->itemFromIndex(i);
        if (item->itemType() == DFMSideBarItem::SidebarItem && item->groupName() == group) {
            index = i;
        } else if (item->groupName() != group && index != -1) {
            // already found the group and already leaved the group
            break;
        }
    }

    return index;
}

void DFMSideBar::openItemEditor(int index) const
{
    m_sidebarView->openPersistentEditor(m_sidebarModel->index(index, 0));
}

QSet<QString> DFMSideBar::disableUrlSchemes() const
{
    return QSet<QString>();
}

void DFMSideBar::setContextMenuEnabled(bool enabled)
{
    m_contextMenuEnabled = enabled;
}

void DFMSideBar::setDisableUrlSchemes(const QSet<QString> &schemes)
{
    //
}

DUrlList DFMSideBar::savedItemOrder(const QString &groupName) const
{
    DUrlList list;

    QStringList savedList = DFMApplication::genericSetting()->value(SIDEBAR_ITEMORDER_KEY, groupName).toStringList();
    for (const QString & item : savedList) {
        list << DUrl(item);
    }

    return list;
}

void DFMSideBar::saveItemOrder(const QString &groupName) const
{
    QVariantList list;

    for (int i = 0; i < m_sidebarModel->rowCount(); i++) {
        DFMSideBarItem * item = m_sidebarModel->itemFromIndex(m_sidebarModel->index(i, 0));
        if (item->itemType() == DFMSideBarItem::SidebarItem && item->groupName() == groupName) {
            list << QVariant(item->url());
        }
    }

    DFMApplication::genericSetting()->setValue(SIDEBAR_ITEMORDER_KEY, groupName, list);
}

QString DFMSideBar::groupName(DFMSideBar::GroupName group)
{
    Q_ASSERT(group != Unknow);

    switch (group) {
    case Common:
        return "common";
    case Device:
        return "device";
    case Bookmark:
        return "bookmark";
    case Network:
        return "network";
    case Tag:
        return "tag";
    case Other: // deliberate
    default:
        break;
    }

    return QString();
}

DFMSideBar::GroupName DFMSideBar::groupFromName(const QString &name)
{
    if (name.isEmpty()) {
        return Other;
    }

    switch (name.toLatin1().at(0)) {
    case 'c':
        if (name == QStringLiteral("common")) {
            return Common;
        }

        break;
    case 'd':
        if (name == QStringLiteral("device")) {
            return Device;
        }

        break;
    case 'b':
        if (name == QStringLiteral("bookmark")) {
            return Bookmark;
        }

        break;
    case 'n':
        if (name == QStringLiteral("network")) {
            return Network;
        }

        break;
    case 't':
        if (name == QStringLiteral("tag")) {
            return Tag;
        }

        break;
    default:
        break;
    }

    return Unknow;
}

void DFMSideBar::onItemActivated(const QModelIndex &index)
{
    DFMSideBarItem * item = m_sidebarModel->itemFromIndex(index);
    QString identifierStr = item->registeredHandler(SIDEBAR_ID_INTERNAL_FALLBACK);

    QScopedPointer<DFMSideBarItemInterface> interface(DFMSideBarManager::instance()->createByIdentifier(identifierStr));
    if (interface) {
        interface->cdAction(this, item);
    }
}

void DFMSideBar::onContextMenuRequested(const QPoint &pos)
{
    if (!m_contextMenuEnabled) return;

    QModelIndex modelIndex = m_sidebarView->indexAt(pos);
    if (!modelIndex.isValid()) {
        return;
    }

    DFMSideBarItem *item = m_sidebarModel->itemFromIndex(modelIndex);
    QString identifierStr = item->registeredHandler(SIDEBAR_ID_INTERNAL_FALLBACK);

    QScopedPointer<DFMSideBarItemInterface> interface(DFMSideBarManager::instance()->createByIdentifier(identifierStr));
    QMenu *menu = nullptr;

    if (interface) {
        menu = interface->contextMenu(this, item);
        if (menu) {
            menu->exec(this->mapToGlobal(pos));
            menu->deleteLater();
        }
    }

    return;
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
}

void DFMSideBar::initModelData()
{
    // register meta type for DUrl, since we use it in item view DnD operation.
    qRegisterMetaTypeStreamOperators<DUrl>("DUrl");

    static QList<DFMSideBar::GroupName> groups = {
        GroupName::Common, GroupName::Device, GroupName::Bookmark, GroupName::Network, GroupName::Tag
    };

    foreach (const DFMSideBar::GroupName &groupType, groups) {
#ifdef DISABLE_TAG_SUPPORT
        if (groupType == DFMSideBar::GroupName::Tag) continue;
#endif // DISABLE_TAG_SUPPORT

        m_sidebarModel->appendRow(DFMSideBarItem::createSeparatorItem(groupName(groupType)));
        addGroupItems(groupType);
    }

    // init done, then we should update the separator visible state.
    updateSeparatorVisibleState();
}

void DFMSideBar::initConnection()
{
    // do `cd` work
    connect(m_sidebarView, &QListView::activated, this, &DFMSideBar::onItemActivated);

    // we need single click also trigger activated()
    connect(m_sidebarView, &QListView::clicked, this, &DFMSideBar::onItemActivated);

    // context menu
    connect(m_sidebarView, &QListView::customContextMenuRequested, this, &DFMSideBar::onContextMenuRequested);

    // so no extra separator if a group is empty.
    // since we do this, ensure we do initConnection() after initModelData().
    connect(m_sidebarModel, &QStandardItemModel::rowsInserted, this, &DFMSideBar::updateSeparatorVisibleState);
    connect(m_sidebarModel, &QStandardItemModel::rowsRemoved, this, &DFMSideBar::updateSeparatorVisibleState);
    connect(m_sidebarModel, &QStandardItemModel::rowsMoved, this, &DFMSideBar::updateSeparatorVisibleState);

    initBookmarkConnection();
    initDeviceConnection();
    initTagsConnection();
}

void DFMSideBar::initBookmarkConnection()
{
    DAbstractFileWatcher *bookmarkWatcher = DFileService::instance()->createFileWatcher(this, DUrl(BOOKMARK_ROOT), this);
    bookmarkWatcher->startWatcher();

    connect(bookmarkWatcher, &DAbstractFileWatcher::subfileCreated, this,
    [this](const DUrl & url) {
        const QString &groupNameStr = groupName(Bookmark);
        this->addItem(DFMSideBarBookmarkItemHandler::createItem(url), groupNameStr);
        this->saveItemOrder(groupNameStr);
    });

    connect(bookmarkWatcher, &DAbstractFileWatcher::fileDeleted, this,
    [this](const DUrl & url) {
        qDebug() << url;
        int index = findItem(url, groupName(Bookmark));
        if (index >= 0) {
            m_sidebarModel->removeRow(index);
            this->saveItemOrder(groupName(Bookmark));
        }
    });

    connect(bookmarkWatcher, &DAbstractFileWatcher::fileMoved, this,
    [this](const DUrl & source, const DUrl & target) {
        int index = findItem(source, groupName(Bookmark));
        if (index > 0) {
            DFMSideBarItem * item = m_sidebarModel->itemFromIndex(index);
            if (item) {
                item->setUrl(target);
                this->saveItemOrder(groupName(Bookmark));
            }
        }
    });
}

void DFMSideBar::initDeviceConnection()
{
    DAbstractFileWatcher *devicesWatcher = DFileService::instance()->createFileWatcher(nullptr, DUrl(DEVICE_ROOT), this);
    Q_CHECK_PTR(devicesWatcher);
    devicesWatcher->startWatcher();

    m_udisks2DiskManager.reset(new DDiskManager);
    m_udisks2DiskManager->setWatchChanges(true);

    auto devicesInfo = DFileService::instance()->getChildren(this, DUrl(DEVICE_ROOT),
                        QStringList(), QDir::AllEntries);
    for (const DAbstractFileInfoPointer &info : devicesInfo) {
        QString devs(info->fileUrl().path());
        devs.replace("/dev/", "/org/freedesktop/UDisks2/block_devices/");
        QScopedPointer<DBlockDevice> blkdev(DDiskManager::createBlockDevice(devs));
        QScopedPointer<DDiskDevice> drv(DDiskManager::createDiskDevice(blkdev->drive()));
        if (drv->mediaCompatibility().join(' ').contains("optical")) {
            continue;
        }
        addItem(DFMSideBarDeviceItemHandler::createItem(info->fileUrl()), groupName(Device));
    }

    // optical device..
    for (auto& blks : m_udisks2DiskManager->blockDevices()) {
        QScopedPointer<DBlockDevice> blk(DDiskManager::createBlockDevice(blks));
        QScopedPointer<DDiskDevice> drv(DDiskManager::createDiskDevice(blk->drive()));
        if (drv->mediaCompatibility().join(' ').contains("optical")) {
            addItem(DFMSideBarOpticalItemHandler::createItem(DUrl::fromDeviceId(blk->device())), groupName(Device));
        }
    }

    // New device/volume added.
    connect(devicesWatcher, &DAbstractFileWatcher::subfileCreated, this, [this](const DUrl & url) {
        QString devs(url.path());
        devs.replace("/dev/", "/org/freedesktop/UDisks2/block_devices/");
        QScopedPointer<DBlockDevice> blkdev(DDiskManager::createBlockDevice(devs));
        QScopedPointer<DDiskDevice> drv(DDiskManager::createDiskDevice(blkdev->drive()));
        if (drv->mediaCompatibility().join(' ').contains("optical")) {
            return;
        }
        addItem(DFMSideBarDeviceItemHandler::createItem(url), groupName(Device));
    });

    connect(m_udisks2DiskManager.data(), &DDiskManager::blockDeviceAdded, this, [this](const QString & s) {
        QScopedPointer<DBlockDevice> blk(DDiskManager::createBlockDevice(s));
        QScopedPointer<DDiskDevice> drv(DDiskManager::createDiskDevice(blk->drive()));
        if (drv->mediaCompatibility().join(' ').contains("optical")) {
            addItem(DFMSideBarDeviceItemHandler::createItem(DUrl::fromDeviceId(blk->device())), groupName(Device));
        }
    });

//    connect(m_udisks2DiskManager.data(), &DDiskManager::opticalChanged, this, [this](const QString & path) {
//        QScopedPointer<DDiskDevice> drv(DDiskManager::createDiskDevice(path));
//        for (auto blks : m_udisks2DiskManager->blockDevices()) {
//            QScopedPointer<DBlockDevice> blkdev(DDiskManager::createBlockDevice(blks));
//            if (path == blkdev->drive()) {
//                DUrl url = DUrl::fromDeviceId(blkdev->device());
//                DFMSideBarItem *item = group->findItem(url);
//                DFMSideBarOpticalDevItem *optdevitem = qobject_cast<DFMSideBarOpticalDevItem*>(item);
//                if (optdevitem) {
//                    optdevitem->unmountButton->setVisible(drv->mediaAvailable());
//                    optdevitem->reloadLabel();
//                }
//            }
//        }
//    });

    // Device/volume get mounted/unmounted
    connect(devicesWatcher, &DAbstractFileWatcher::fileAttributeChanged, this, [this](const DUrl & url) {
        int index = findItem(url, groupName(Device));
        DFMSideBarItem *item = m_sidebarModel->itemFromIndex(index);
        DViewItemActionList actionList = item->actionList(Qt::RightEdge);
        DAbstractFileInfoPointer pointer = DFileService::instance()->createFileInfo(nullptr, url);
        if (item && !actionList.isEmpty()) {
            const QVariantHash &extensionInfo = pointer->extraProperties();

            bool isMounted = extensionInfo.value("isMounted", false).toBool();
            bool canUnmount = extensionInfo.value("canUnmount", true).toBool();
            bool isRemovable = extensionInfo.value("isRemovable", true).toBool();
            bool canEject = extensionInfo.value("canEject", true).toBool();
            bool isOptical = extensionInfo.value("optical", true).toBool();

            if (isRemovable || isOptical) {
                actionList.first()->setVisible(canEject || canUnmount);
            } else {
                actionList.first()->setVisible(isMounted && canUnmount);
            }
        }
    });

    // Device/volume get removed.
//    connect(devicesWatcher, &DAbstractFileWatcher::fileDeleted, this, [this](const DUrl & url) {
////        this->removeItem(url, groupName(Device));
//        DFMSideBarItem *item = group->findItem(url);
//        if (item && !item->inherits(DFMSideBarOpticalDevItem::staticMetaObject.className())) {
//            q->removeItem(item);
//        }
//    });
    connect(m_udisks2DiskManager.data(), &DDiskManager::blockDeviceRemoved, this, [this](const QString & s) {
        QString devs(s);
        devs.replace("/org/freedesktop/UDisks2/block_devices/", "/dev/");
        DUrl url = DUrl::fromDeviceId(devs);
        this->removeItem(url, groupName(Device));
    });

//    // Device/volume get renamed.
//    q->connect(devices_watcher, &DAbstractFileWatcher::fileMoved, group, [group, q](const DUrl & fromUrl, const DUrl & toUrl) {
//        Q_UNUSED(fromUrl);
//        QUrlQuery query(toUrl);
//        QString newName = query.queryItemValue("new_name");
//        DUrl url = toUrl.adjusted(DUrl::RemoveQuery);
//        DFMSideBarItem *item = group->findItem(url);
//        // item could be NULL for a labelled optical media
//        if (!item) {
//            return;
//        }
//        DAbstractFileInfoPointer fileInfo = DFileService::instance()->createFileInfo(q, url);
//        if (fileInfo) {
//            if (!newName.isEmpty()) {
//                item->setText(newName);
//            } else {
//                item->setText(fileInfo->fileDisplayName());
//            }
//        }
    //    });
}

void DFMSideBar::initTagsConnection()
{
#ifdef DISABLE_TAG_SUPPORT
    return;
#endif

    DAbstractFileWatcher *tagsWatcher = DFileService::instance()->createFileWatcher(this, DUrl(TAG_ROOT), this);
    Q_CHECK_PTR(tagsWatcher);
    tagsWatcher->startWatcher();

    QString groupNameStr(groupName(Tag));

    // New tag added.
    connect(tagsWatcher, &DAbstractFileWatcher::subfileCreated, this, [this, groupNameStr](const DUrl & url) {
        this->addItem(DFMSideBarTagItemHandler::createItem(url), groupNameStr);
        this->saveItemOrder(groupNameStr);
    });

    // Tag get removed.
    connect(tagsWatcher, &DAbstractFileWatcher::fileDeleted, this, [this, groupNameStr](const DUrl & url) {
        this->removeItem(url, groupNameStr);
        this->saveItemOrder(groupNameStr);
    });

//    // Tag got rename
//    q->connect(tagsWatcher, &DAbstractFileWatcher::fileMoved, group,
//    [this, group, q](const DUrl & source, const DUrl & target) {
//        DFMSideBarItem *item = q->itemAt(source);
//        if (item) {
//            item->setUrl(target);
//            group->saveItemOrder();
//        }
//    });

//    // Tag changed color
//    q->connect(tagsWatcher, &DAbstractFileWatcher::fileAttributeChanged, group, [group](const DUrl & url) {
//        DFMSideBarItem *item = group->findItem(url);
//        item->setIconFromThemeConfig("BookmarkItem." + TagManager::instance()->getTagColorName(url.tagName()));
//    });
}

void DFMSideBar::updateSeparatorVisibleState()
{
    QString lastGroupName = "__not_existed_group";
    int lastGroupItemCount = 0;
    int lastSeparatorIndex = -1;

    for (int i = 0; i < m_sidebarModel->rowCount(); i++) {
        DFMSideBarItem * item = m_sidebarModel->itemFromIndex(i);
        if (item->groupName() != lastGroupName) {
            if (item->itemType() == DFMSideBarItem::Separator) {
                m_sidebarView->setRowHidden(i, lastGroupItemCount == 0);
                lastSeparatorIndex = i;
                lastGroupItemCount = 0;
                lastGroupName = item->groupName();
            }
        } else {
            if (item->itemType() == DFMSideBarItem::SidebarItem) {
                lastGroupItemCount++;
            }
        }
    }

    // hide the last one if last group is empty
    if (lastGroupItemCount == 0) {
        m_sidebarView->setRowHidden(lastSeparatorIndex, true);
    }
}

void DFMSideBar::addGroupItems(DFMSideBar::GroupName groupType)
{
    const QString &groupNameStr = groupName(groupType);
    switch (groupType) {
    case GroupName::Common:
        appendItem(DFMSideBarDefaultItemHandler::createItem("Recent"), groupNameStr);
        appendItem(DFMSideBarDefaultItemHandler::createItem("Home"), groupNameStr);
        appendItem(DFMSideBarDefaultItemHandler::createItem("Desktop"), groupNameStr);
        appendItem(DFMSideBarDefaultItemHandler::createItem("Videos"), groupNameStr);
        appendItem(DFMSideBarDefaultItemHandler::createItem("Music"), groupNameStr);
        appendItem(DFMSideBarDefaultItemHandler::createItem("Pictures"), groupNameStr);
        appendItem(DFMSideBarDefaultItemHandler::createItem("Documents"), groupNameStr);
        appendItem(DFMSideBarDefaultItemHandler::createItem("Downloads"), groupNameStr);
        appendItem(DFMSideBarDefaultItemHandler::createItem("Trash"), groupNameStr);
        break;
    case GroupName::Device:
        appendItem(DFMSideBarDefaultItemHandler::createItem("Computer"), groupNameStr);
        appendItem(DFMSideBarDefaultItemHandler::createItem("System Disk"), groupNameStr);
        break;
    case GroupName::Bookmark: {
        QList<DAbstractFileInfoPointer> bookmarkInfos = DFileService::instance()->getChildren(this, DUrl(BOOKMARK_ROOT),
                                                         QStringList(), QDir::AllEntries);
        QList<DFMSideBarItem *> unsortedList;
        for (const DAbstractFileInfoPointer &info : bookmarkInfos) {
            unsortedList << DFMSideBarBookmarkItemHandler::createItem(info->fileUrl());
        }
        appendItemWithOrder(unsortedList, savedItemOrder(groupNameStr), groupNameStr);
        break;
    }
    case GroupName::Network:
        appendItem(DFMSideBarDefaultItemHandler::createItem("Network"), groupNameStr);
        appendItem(DFMSideBarDefaultItemHandler::createItem("UserShare"), groupNameStr);
        break;
    case GroupName::Tag: {
        auto tag_infos = DFileService::instance()->getChildren(this, DUrl(TAG_ROOT),
                              QStringList(), QDir::AllEntries);
        QList<DFMSideBarItem *> unsortedList;
        for (const DAbstractFileInfoPointer &info : tag_infos) {
            unsortedList << DFMSideBarTagItemHandler::createItem(info->fileUrl());
        }
        appendItemWithOrder(unsortedList, savedItemOrder(groupNameStr), groupNameStr);
        break;
    }
    default:
        break;
    }
}

void DFMSideBar::insertItem(int index, DFMSideBarItem *item, const QString &groupName)
{
    item->setGroupName(groupName);
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
    item->setGroupName(groupName);
    m_sidebarModel->appendRow(item);
}

void DFMSideBar::appendItemWithOrder(QList<DFMSideBarItem *> &list, const DUrlList &order, const QString &groupName)
{
    DUrlList urlList;

    for (const DFMSideBarItem* item : list) {
        urlList << item->url();
    }

    for (const DUrl & url: order) {
        int idx = urlList.indexOf(url);
        if (idx >= 0) {
            urlList.removeAt(idx);
            this->appendItem(list.takeAt(idx), groupName);
        }
    }

    for (DFMSideBarItem * item: list) {
        this->appendItem(item, groupName);
    }
}

DFM_END_NAMESPACE
