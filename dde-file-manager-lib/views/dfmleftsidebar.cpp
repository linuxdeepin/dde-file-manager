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
#include "dfmleftsidebar.h"

#include "dfmapplication.h"
#include "dfmsettings.h"
#include "dabstractfilewatcher.h"
#include "dfilemanagerwindow.h"
#include "dfileservices.h"
#include "singleton.h"

#include "views/dfmsidebarview.h"
#include "models/dfmsidebarmodel.h"
#include "dfmsidebaritemdelegate.h"
#include "dfmleftsidebaritem.h"

#include <QVBoxLayout>
#include <QDebug>

#include <ddiskmanager.h>
#include <ddiskdevice.h>
#include <dblockdevice.h>
#include <QMenu>

#define SIDEBAR_ITEMORDER_KEY "SideBar/ItemOrder"

DFM_USE_NAMESPACE

DFMLeftSideBar::DFMLeftSideBar(QWidget *parent)
    : QWidget(parent),
      m_sidebarView(new DFMSideBarView(this)),
      m_sidebarModel(new DFMSideBarModel(this))
{
    // init view.
    m_sidebarView->setModel(m_sidebarModel);
    m_sidebarView->setItemDelegate(new DFMSideBarItemDelegate(this));
    m_sidebarView->setContextMenuPolicy(Qt::CustomContextMenu);

    initUI();
    initModelData();
    initConnection();
}

void DFMLeftSideBar::setCurrentUrl(const DUrl &url)
{
    Q_UNUSED(url);
}

int DFMLeftSideBar::addItem(DFMLeftSideBarItem *item, const QString &group)
{
    int lastAtGroup = findLastItem(group);
    lastAtGroup++; // append after the last item
    m_sidebarModel->insertRow(lastAtGroup, item);

    return lastAtGroup;
}

bool DFMLeftSideBar::removeItem(const DUrl &url, const QString &group)
{
    int index = findItem(url, group);
    bool succ = false;
    if (index >= 0) {
        succ = m_sidebarModel->removeRow(index);
    }

    return succ;
}

int DFMLeftSideBar::findItem(const DUrl &url, const QString &group) const
{
    for (int i = 0; i < m_sidebarModel->rowCount(); i++) {
        DFMLeftSideBarItem * item = m_sidebarModel->itemFromIndex(i);
        if (item->itemType() == DFMLeftSideBarItem::SidebarItem && item->groupName() == group) {
            if (item->url() == url) {
                return i;
            }
        }
    }

    return -1;
}

int DFMLeftSideBar::findLastItem(const QString &group) const
{
    int index = -1;
    for (int i = 0; i < m_sidebarModel->rowCount(); i++) {
        DFMLeftSideBarItem * item = m_sidebarModel->itemFromIndex(i);
        if (item->itemType() == DFMLeftSideBarItem::SidebarItem && item->groupName() == group) {
            index = i;
        } else if (item->groupName() != group && index != -1) {
            // already found the group and already leaved the group
            break;
        }
    }

    return index;
}

DUrlList DFMLeftSideBar::savedItemOrder(const QString &groupName) const
{
    DUrlList list;

    QStringList savedList = DFMApplication::genericSetting()->value(SIDEBAR_ITEMORDER_KEY, groupName).toStringList();
    for (const QString & item : savedList) {
        list << DUrl(item);
    }

    return list;
}

void DFMLeftSideBar::saveItemOrder(const QString &groupName) const
{
    QVariantList list;

    for (int i = 0; i < m_sidebarModel->rowCount(); i++) {
        DFMLeftSideBarItem * item = m_sidebarModel->itemFromIndex(m_sidebarModel->index(i, 0));
        if (item->itemType() == DFMLeftSideBarItem::SidebarItem && item->groupName() == groupName) {
            list << QVariant(item->url());
        }
    }

    DFMApplication::genericSetting()->setValue(SIDEBAR_ITEMORDER_KEY, groupName, list);
}

QString DFMLeftSideBar::groupName(DFMLeftSideBar::GroupName group)
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

DFMLeftSideBar::GroupName DFMLeftSideBar::groupFromName(const QString &name)
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

void DFMLeftSideBar::onItemActivated(const QModelIndex &index)
{
    DFMLeftSideBarItem * item = m_sidebarModel->itemFromIndex(index);
    if (item) {
        qDebug() << item->url();
        // simple switch, can be changed to plugin-based one.
        switch (item->cdActionType()) {
        case DFMLeftSideBarItem::ChangeDirectory: {
            DFileManagerWindow *wnd = qobject_cast<DFileManagerWindow *>(this->topLevelWidget());
            wnd->cd(item->url()); // don't `setChecked` here, wait for a signal.
            break;
        }
        case DFMLeftSideBarItem::MountPartitionThenCd: {
            QVariantHash info = DFileService::instance()->createFileInfo(this, item->url())->extraProperties();

            if (info.value("isMounted", false).toBool()) {
                DFileManagerWindow *wnd = qobject_cast<DFileManagerWindow *>(topLevelWidget());
                wnd->cd(item->url());
            } else if (info.value("canMount", false).toBool()) {
                DUrl newUrl;
                newUrl.setQuery(info.value("deviceId").toString());
                AppController::instance()->actionOpenDisk(dMakeEventPointer<DFMUrlBaseEvent>(this, newUrl));
            }
            break;
        }
        default:
            break;
        }
    }
}

void DFMLeftSideBar::onContextMenuRequested(const QPoint &pos)
{
    QModelIndex modelIndex = m_sidebarView->indexAt(pos);
    if (!modelIndex.isValid()) {
        return;
    }

    DFMLeftSideBarItem *item = m_sidebarModel->itemFromIndex(modelIndex);
    Q_UNUSED(item);

    QMenu *menu = new QMenu();

    menu->addAction(QObject::tr("Open in new window"));

    menu->addAction(QObject::tr("Open in new tab"));

    menu->exec(this->mapToGlobal(pos));
}

void DFMLeftSideBar::initUI()
{
    // init layout.
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_sidebarView);

    layout->setMargin(0);
    layout->setSpacing(0);

    this->setMaximumWidth(200);
    this->setFocusProxy(m_sidebarView);
}

void DFMLeftSideBar::initModelData()
{
    // register meta type for DUrl, since we use it in item view DnD operation.
    qRegisterMetaTypeStreamOperators<DUrl>("DUrl");

    static QList<DFMLeftSideBar::GroupName> groups = {
        GroupName::Common, GroupName::Device, GroupName::Bookmark, GroupName::Network, GroupName::Tag
    };

    foreach (const DFMLeftSideBar::GroupName &groupType, groups) {
#ifdef DISABLE_TAG_SUPPORT
        if (groupType == DFMLeftSideBar::GroupName::Tag) continue;
#endif // DISABLE_TAG_SUPPORT

        m_sidebarModel->appendRow(DFMLeftSideBarItem::createSeparatorItem(groupName(groupType)));
        addGroupItems(groupType);
    }

    // init done, then we should update the separator visible state.
    updateSeparatorVisibleState();
}

void DFMLeftSideBar::initConnection()
{
    // do `cd` work
    connect(m_sidebarView, &QListView::activated, this, &DFMLeftSideBar::onItemActivated);

    // context menu
    connect(m_sidebarView, &QListView::customContextMenuRequested, this, &DFMLeftSideBar::onContextMenuRequested);

    // so no extra separator if a group is empty.
    // since we do this, ensure we do initConnection() after initModelData().
    connect(m_sidebarModel, &QStandardItemModel::rowsInserted, this, &DFMLeftSideBar::updateSeparatorVisibleState);
    connect(m_sidebarModel, &QStandardItemModel::rowsRemoved, this, &DFMLeftSideBar::updateSeparatorVisibleState);
    connect(m_sidebarModel, &QStandardItemModel::rowsMoved, this, &DFMLeftSideBar::updateSeparatorVisibleState);

    initBookmarkConnection();
    initDeviceConnection();
}

void DFMLeftSideBar::initBookmarkConnection()
{
    DAbstractFileWatcher *bookmarkWatcher = DFileService::instance()->createFileWatcher(this, DUrl(BOOKMARK_ROOT), this);
    bookmarkWatcher->startWatcher();

    connect(bookmarkWatcher, &DAbstractFileWatcher::subfileCreated, this,
    [this](const DUrl & url) {
        const QString &groupNameStr = groupName(Bookmark);
        this->addItem(DFMLeftSideBarItem::createBookmarkItem(url, groupNameStr), groupNameStr);
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
            DFMLeftSideBarItem * item = m_sidebarModel->itemFromIndex(index);
            if (item) {
                item->setUrl(target);
                this->saveItemOrder(groupName(Bookmark));
            }
        }
    });
}

void DFMLeftSideBar::initDeviceConnection()
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
        this->addItem(DFMLeftSideBarItem::createDeviceItem(info->fileUrl(), groupName(Device)), groupName(Device));
//        group->appendItem(new DFMSideBarDeviceItem(info->fileUrl()));
    }

    // optical device..
    for (auto& blks : m_udisks2DiskManager->blockDevices()) {
        QScopedPointer<DBlockDevice> blk(DDiskManager::createBlockDevice(blks));
        QScopedPointer<DDiskDevice> drv(DDiskManager::createDiskDevice(blk->drive()));
        if (drv->mediaCompatibility().join(' ').contains("optical")) {
            this->addItem(DFMLeftSideBarItem::createDeviceItem(DUrl::fromDeviceId(blk->device()), groupName(Device)),
                          groupName(Device));
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
        this->addItem(DFMLeftSideBarItem::createDeviceItem(url, groupName(Device)),
                      groupName(Device));
//        group->appendItem(new DFMSideBarDeviceItem(url));
    });

    connect(m_udisks2DiskManager.data(), &DDiskManager::blockDeviceAdded, this, [this](const QString & s) {
        QScopedPointer<DBlockDevice> blk(DDiskManager::createBlockDevice(s));
        QScopedPointer<DDiskDevice> drv(DDiskManager::createDiskDevice(blk->drive()));
        if (drv->mediaCompatibility().join(' ').contains("optical")) {
            this->addItem(DFMLeftSideBarItem::createDeviceItem(DUrl::fromDeviceId(blk->device()), groupName(Device))
                          , groupName(Device));
//            group->appendItem(new DFMSideBarOpticalDevItem(DUrl::fromDeviceId(blk->device())));
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
//    q->connect(devices_watcher, &DAbstractFileWatcher::fileAttributeChanged, group, [group](const DUrl & url) {
//        DFMSideBarItem *item = group->findItem(url);
//        DAbstractFileInfoPointer pointer = fileService->createFileInfo(item, url);
//        if (item && !item->inherits(DFMSideBarOpticalDevItem::staticMetaObject.className())) {
//            DFMSideBarDeviceItem *casted = qobject_cast<DFMSideBarDeviceItem *>(item);
//            const QVariantHash &extensionInfo = pointer->extraProperties();

//            bool isMounted = extensionInfo.value("isMounted", false).toBool();
//            bool canUnmount = extensionInfo.value("canUnmount", true).toBool();
//            bool isRemovable = extensionInfo.value("isRemovable", true).toBool();
//            bool canEject = extensionInfo.value("canEject", true).toBool();

//            if (casted) {
//                if (isRemovable) {
//                    casted->unmountButton->setVisible(canEject || canUnmount);
//                } else {
//                    casted->unmountButton->setVisible(isMounted && canUnmount);
//                }
//            } else {
//                DFMSideBarOpticalDevItem *optdevitem = qobject_cast<DFMSideBarOpticalDevItem *>(item);
//                optdevitem->unmountButton->setVisible(canEject || canUnmount);
//            }
//        }
//    });

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

void DFMLeftSideBar::updateSeparatorVisibleState()
{
    QString lastGroupName = "__not_existed_group";
    int lastGroupItemCount = 0;
    int lastSeparatorIndex = -1;

    for (int i = 0; i < m_sidebarModel->rowCount(); i++) {
        DFMLeftSideBarItem * item = m_sidebarModel->itemFromIndex(i);
        if (item->groupName() != lastGroupName) {
            if (item->itemType() == DFMLeftSideBarItem::Separator) {
                m_sidebarView->setRowHidden(i, lastGroupItemCount == 0);
                lastSeparatorIndex = i;
                lastGroupItemCount = 0;
                lastGroupName = item->groupName();
            }
        } else {
            if (item->itemType() == DFMLeftSideBarItem::SidebarItem) {
                lastGroupItemCount++;
            }
        }
    }

    // hide the last one if last group is empty
    if (lastGroupItemCount == 0) {
        m_sidebarView->setRowHidden(lastSeparatorIndex, true);
    }
}

void DFMLeftSideBar::addGroupItems(DFMLeftSideBar::GroupName groupType)
{
    const QString &groupNameStr = groupName(groupType);
    switch (groupType) {
    case GroupName::Common:
        m_sidebarModel->appendRow(DFMLeftSideBarItem::createSystemPathItem("Recent", groupNameStr));
        m_sidebarModel->appendRow(DFMLeftSideBarItem::createSystemPathItem("Home", groupNameStr));
        m_sidebarModel->appendRow(DFMLeftSideBarItem::createSystemPathItem("Desktop", groupNameStr));
        m_sidebarModel->appendRow(DFMLeftSideBarItem::createSystemPathItem("Videos", groupNameStr));
        m_sidebarModel->appendRow(DFMLeftSideBarItem::createSystemPathItem("Music", groupNameStr));
        m_sidebarModel->appendRow(DFMLeftSideBarItem::createSystemPathItem("Pictures", groupNameStr));
        m_sidebarModel->appendRow(DFMLeftSideBarItem::createSystemPathItem("Documents", groupNameStr));
        m_sidebarModel->appendRow(DFMLeftSideBarItem::createSystemPathItem("Downloads", groupNameStr));
        m_sidebarModel->appendRow(DFMLeftSideBarItem::createSystemPathItem("Trash", groupNameStr));
        break;
    case GroupName::Device:
        m_sidebarModel->appendRow(DFMLeftSideBarItem::createSystemPathItem("Computer", groupNameStr));
        m_sidebarModel->appendRow(DFMLeftSideBarItem::createSystemPathItem("System Disk", groupNameStr));
        break;
    case GroupName::Bookmark: {
        QList<DAbstractFileInfoPointer> bookmarkInfos = DFileService::instance()->getChildren(this, DUrl(BOOKMARK_ROOT),
                                                         QStringList(), QDir::AllEntries);
        QList<DFMLeftSideBarItem *> unsortedList;
        for (const DAbstractFileInfoPointer &info : bookmarkInfos) {
            unsortedList << DFMLeftSideBarItem::createBookmarkItem(info->fileUrl(), groupNameStr);
        }
        appendItemWithOrder(unsortedList, savedItemOrder(groupNameStr));
        break;
    }
    case GroupName::Network:
        m_sidebarModel->appendRow(DFMLeftSideBarItem::createSystemPathItem("Network", groupNameStr));
        m_sidebarModel->appendRow(DFMLeftSideBarItem::createSystemPathItem("UserShare", groupNameStr));
        break;
    case GroupName::Tag: {
        auto tag_infos = DFileService::instance()->getChildren(this, DUrl(TAG_ROOT),
                              QStringList(), QDir::AllEntries);
        QList<DFMLeftSideBarItem *> unsortedList;
        for (const DAbstractFileInfoPointer &info : tag_infos) {
            unsortedList << DFMLeftSideBarItem::createTagItem(info->fileUrl(), groupNameStr);
        }
        appendItemWithOrder(unsortedList, savedItemOrder(groupNameStr));
        break;
    }
    default:
        break;
    }
}

void DFMLeftSideBar::appendItemWithOrder(QList<DFMLeftSideBarItem *> &list, const DUrlList &order)
{
    DUrlList urlList;

    for (const DFMLeftSideBarItem* item : list) {
        urlList << item->url();
    }

    for (const DUrl & url: order) {
        int idx = urlList.indexOf(url);
        if (idx >= 0) {
            urlList.removeAt(idx);
            m_sidebarModel->appendRow(list.takeAt(idx));
        }
    }

    for (DFMLeftSideBarItem * item: list) {
        m_sidebarModel->appendRow(item);
    }
}
