/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     zccrs <zccrs@live.com>
 *
 * Maintainer: zccrs <zhangjide@deepin.com>
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

#include "app/define.h"
#include "views/dfmsidebaritemgroup.h"
#include "views/dfmsidebarbookmarkitem.h"
#include "views/dfmsidebarhomeitem.h"
#include "views/dfmsidebardefaultitem.h"
#include "views/dfmsidebartrashitem.h"
#include "views/dfmsidebartagitem.h"
#include "views/dfmsidebardeviceitem.h"
#include "views/dfmsidebaropticaldevitem.h"
#include "views/dfilemanagerwindow.h"
#include "views/dfmsidebarnetworkitem.h"
#include "views/dfmsidebarrecentitem.h"
#include "views/dfmsidebarrootitem.h"
#include "controllers/bookmarkmanager.h"
#include "deviceinfo/udisklistener.h"

#include <DThemeManager>
#include <dboxwidget.h>

#include <QVBoxLayout>
#include <QTimer>
#include <QScrollBar>
#include <QScroller>

#include "dabstractfilewatcher.h"
#include "ddiskmanager.h"
#include "dblockdevice.h"
#include "ddiskdevice.h"
#include "dfmapplication.h"
#include "tag/tagmanager.h"
#include "singleton.h"


DWIDGET_USE_NAMESPACE

DFM_BEGIN_NAMESPACE

class DFMSideBarPrivate
{
    Q_DECLARE_PUBLIC(DFMSideBar)

public:
    DFMSideBarPrivate(DFMSideBar *qq);

    bool contextMenuEnabled = true;

    DFMSideBar *q_ptr = nullptr;
    QBoxLayout *mainLayout;
    DBoxWidget *mainLayoutHolder;
    QSet<QString> disabledSchemes;
    QMap<QString, DFMSideBarItemGroup *> groupNameMap;
    DFMSideBarItem *lastCheckedItem = nullptr; //< managed by setCurrentUrl()
    QScopedPointer<DDiskManager> dskm;
    void groupConnectionRegister(DFMSideBarItemGroup *group);
    void appendListWithOrder(QList<DFMSideBarItem *> itemList, DFMSideBarItemGroup* groupPointer) const;

    void _q_onItemDragRelease(QPoint cursorPos, Qt::DropAction action, const DFMSideBarItem* item);

private:
    void initUI();
    void initBookmarkConnection();
    void initMountedVolumes();
    void initUserShareItem();
    void initTagsConnection();
    void initRecentConnection();
    void setGroupSaveItemOrder(DFMSideBarItemGroup *group, DFMSideBar::GroupName groupType);
    void addItemToGroup(DFMSideBarItemGroup *group, DFMSideBar::GroupName groupType);

    DAbstractFileWatcher *userShareFileWatcher = nullptr; //< managed by initUserShareItem()
};

DFMSideBarPrivate::DFMSideBarPrivate(DFMSideBar *qq)
    : q_ptr(qq)
{
    initUI();
    initBookmarkConnection();
    initMountedVolumes();
    initUserShareItem();
    initTagsConnection();
    initRecentConnection();
}

void DFMSideBarPrivate::groupConnectionRegister(DFMSideBarItemGroup * group)
{
    Q_Q(DFMSideBar);

    Q_CHECK_PTR(group);

    q->connect(group, SIGNAL(itemDragRelease(QPoint, Qt::DropAction, const DFMSideBarItem*)),
               q, SLOT(_q_onItemDragRelease(QPoint, Qt::DropAction, const DFMSideBarItem*)));
}

void DFMSideBarPrivate::appendListWithOrder(QList<DFMSideBarItem *> itemList, DFMSideBarItemGroup *groupPointer) const
{
    if (!groupPointer) {
        return;
    }

    DUrlList savedList = groupPointer->itemOrder();
    DUrlList urlList;

    for (const DFMSideBarItem* item : itemList) {
        urlList << item->url();
    }

    for (const DUrl & url: savedList) {
        int idx = urlList.indexOf(url);
        if (idx >= 0) {
            urlList.removeAt(idx);
            groupPointer->appendItem(itemList.takeAt(idx));
        }
    }

    for (DFMSideBarItem * item: itemList) {
        groupPointer->appendItem(item);
    }
}

void DFMSideBarPrivate::_q_onItemDragRelease(QPoint cursorPos, Qt::DropAction action, const DFMSideBarItem *item)
{
    Q_Q(DFMSideBar);

    DFMSideBarItemGroup * group = qobject_cast<DFMSideBarItemGroup *>(q->sender());
    Q_CHECK_PTR(group);

    QPoint sidebarTopLeftPos = q->mapToGlobal(QPoint(0, 0));
    QRect sidebarGlobalGeometry(sidebarTopLeftPos, q->geometry().size());

    if (item->canDeleteViaDrag() && !sidebarGlobalGeometry.contains(cursorPos) && action == Qt::IgnoreAction) {
        DFileService::instance()->deleteFiles(q, {item->url()}, false);
    }
}

void DFMSideBarPrivate::initUI()
{
    Q_Q(DFMSideBar);
    q->setAcceptDrops(true);
    q->setFocusPolicy(Qt::NoFocus);
    q->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    q->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    q->setViewportMargins(0, 0, -q->verticalScrollBar()->sizeHint().width(), 0);

    // to make QScrollArea scroallable, we need a widget.
    mainLayoutHolder = new DBoxWidget(QBoxLayout::TopToBottom);
    mainLayoutHolder->setObjectName("SidebarMainLayoutHolder");
    mainLayoutHolder->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    q->setWidget(mainLayoutHolder);

    // our main QVBoxLayout, which hold our `DFMSideBarItemGroup`s
    mainLayout = mainLayoutHolder->layout();
    mainLayout->setSpacing(0);
    mainLayout->setMargin(0);
    mainLayout->setAlignment(Qt::AlignTop);
    mainLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);

    static QList<DFMSideBar::GroupName> groups = {
        DFMSideBar::GroupName::Common,
        DFMSideBar::GroupName::Device,
        DFMSideBar::GroupName::Bookmark,
        DFMSideBar::GroupName::Network,
        DFMSideBar::GroupName::Tag
    };

    foreach (const DFMSideBar::GroupName &groupType, groups) {
#ifdef DISABLE_TAG_SUPPORT
        if (groupType == DFMSideBar::GroupName::Tag) continue;
#endif // DISABLE_TAG_SUPPORT

        DFMSideBarItemGroup *group = new DFMSideBarItemGroup(q->groupName(groupType));
        mainLayout->addLayout(group);
        groupConnectionRegister(group);
        setGroupSaveItemOrder(group, groupType);
        addItemToGroup(group, groupType);
        groupNameMap[q->groupName(groupType)] = group;
    }
}

void DFMSideBarPrivate::initBookmarkConnection()
{
    Q_Q(DFMSideBar);

    DFMSideBarItemGroup *group = groupNameMap[q->groupName(DFMSideBar::GroupName::Bookmark)];
    Q_CHECK_PTR(group);

    DAbstractFileWatcher *bookmark_watcher = fileService->createFileWatcher(q_func(), DUrl(BOOKMARK_ROOT), group);
    bookmark_watcher->startWatcher();

    q->connect(bookmark_watcher, &DAbstractFileWatcher::subfileCreated, group, [group](const DUrl & url) {
        group->appendItem(new DFMSideBarBookmarkItem(url));
        group->saveItemOrder();
    });

    q->connect(bookmark_watcher, &DAbstractFileWatcher::fileDeleted, group,
    [this, q, group](const DUrl & url) {
        DFMSideBarItem *item = q->itemAt(url);
        if (item) {
            q->removeItem(item);
            group->saveItemOrder();
        }
    });

    q->connect(bookmark_watcher, &DAbstractFileWatcher::fileMoved, group,
    [this, group, q](const DUrl & source, const DUrl & target) {
        DFMSideBarItem *item = q->itemAt(source);
        if (item) {
            item->setUrl(target);
            group->saveItemOrder();
        }
    });
}

void DFMSideBarPrivate::initMountedVolumes()
{
    Q_Q(DFMSideBar);

    DFMSideBarItemGroup *group = groupNameMap[q->groupName(DFMSideBar::GroupName::Device)];
    Q_CHECK_PTR(group);

    group->setAutoSort(true);

    DAbstractFileWatcher *devices_watcher = fileService->createFileWatcher(q_func(), DUrl(DEVICE_ROOT), group);
    Q_CHECK_PTR(devices_watcher);
    devices_watcher->startWatcher();

    dskm.reset(new DDiskManager);
    dskm->setWatchChanges(true);

    auto devices_info = fileService->getChildren(q_func(), DUrl(DEVICE_ROOT),
                        QStringList(), QDir::AllEntries);
    for (const DAbstractFileInfoPointer &info : devices_info) {
        QString devs(info->fileUrl().path());
        devs.replace("/dev/", "/org/freedesktop/UDisks2/block_devices/");
        QScopedPointer<DBlockDevice> blkdev(DDiskManager::createBlockDevice(devs));
        QScopedPointer<DDiskDevice> drv(DDiskManager::createDiskDevice(blkdev->drive()));
        if (drv->mediaCompatibility().join(' ').contains("optical")) {
            continue;
        }
        group->appendItem(new DFMSideBarDeviceItem(info->fileUrl()));
    }

    for (auto& blks : dskm->blockDevices()) {
        QScopedPointer<DBlockDevice> blk(DDiskManager::createBlockDevice(blks));
        QScopedPointer<DDiskDevice> drv(DDiskManager::createDiskDevice(blk->drive()));
        if (drv->mediaCompatibility().join(' ').contains("optical")) {
            group->appendItem(new DFMSideBarOpticalDevItem(DUrl::fromDeviceId(blk->device())));
        }
    }

    // New device/volume added.
    q->connect(devices_watcher, &DAbstractFileWatcher::subfileCreated, group, [group](const DUrl & url) {
        QString devs(url.path());
        devs.replace("/dev/", "/org/freedesktop/UDisks2/block_devices/");
        QScopedPointer<DBlockDevice> blkdev(DDiskManager::createBlockDevice(devs));
        QScopedPointer<DDiskDevice> drv(DDiskManager::createDiskDevice(blkdev->drive()));
        if (drv->mediaCompatibility().join(' ').contains("optical")) {
            return;
        }
        group->appendItem(new DFMSideBarDeviceItem(url));
    });
    q->connect(dskm.data(), &DDiskManager::blockDeviceAdded, group, [group](const QString & s) {
        QScopedPointer<DBlockDevice> blk(DDiskManager::createBlockDevice(s));
        QScopedPointer<DDiskDevice> drv(DDiskManager::createDiskDevice(blk->drive()));
        if (drv->mediaCompatibility().join(' ').contains("optical")) {
            group->appendItem(new DFMSideBarOpticalDevItem(DUrl::fromDeviceId(blk->device())));
        }
    });
    q->connect(dskm.data(), &DDiskManager::opticalChanged, group, [group, this](const QString & path) {
        QScopedPointer<DDiskDevice> drv(DDiskManager::createDiskDevice(path));
        for (auto blks : dskm->blockDevices()) {
            QScopedPointer<DBlockDevice> blkdev(DDiskManager::createBlockDevice(blks));
            if (path == blkdev->drive()) {
                DUrl url = DUrl::fromDeviceId(blkdev->device());
                DFMSideBarItem *item = group->findItem(url);
                DFMSideBarOpticalDevItem *optdevitem = qobject_cast<DFMSideBarOpticalDevItem*>(item);
                if (optdevitem) {
                    optdevitem->unmountButton->setVisible(drv->mediaAvailable());
                    optdevitem->reloadLabel();
                }
            }
        }
    });

    // Device/volume get mounted/unmounted
    q->connect(devices_watcher, &DAbstractFileWatcher::fileAttributeChanged, group, [group](const DUrl & url) {
        DFMSideBarItem *item = group->findItem(url);
        DAbstractFileInfoPointer pointer = fileService->createFileInfo(item, url);
        if (item && !item->inherits(DFMSideBarOpticalDevItem::staticMetaObject.className())) {
            DFMSideBarDeviceItem *casted = qobject_cast<DFMSideBarDeviceItem *>(item);
            const QVariantHash &extensionInfo = pointer->extraProperties();

            bool isMounted = extensionInfo.value("isMounted", false).toBool();
            bool canUnmount = extensionInfo.value("canUnmount", true).toBool();
            bool isRemovable = extensionInfo.value("isRemovable", true).toBool();
            bool canEject = extensionInfo.value("canEject", true).toBool();

            if (casted) {
                if (isRemovable) {
                    casted->unmountButton->setVisible(canEject || canUnmount);
                } else {
                    casted->unmountButton->setVisible(isMounted && canUnmount);
                }
            } else {
                DFMSideBarOpticalDevItem *optdevitem = qobject_cast<DFMSideBarOpticalDevItem *>(item);
                optdevitem->unmountButton->setVisible(canEject || canUnmount);
            }
        }
    });

    // Device/volume get removed.
    q->connect(devices_watcher, &DAbstractFileWatcher::fileDeleted, group, [group, q](const DUrl & url) {
        DFMSideBarItem *item = group->findItem(url);
        if (item && !item->inherits(DFMSideBarOpticalDevItem::staticMetaObject.className())) {
            q->removeItem(item);
        }
    });
    q->connect(dskm.data(), &DDiskManager::blockDeviceRemoved, group, [group, q](const QString & s) {
        QString devs(s);
        devs.replace("/org/freedesktop/UDisks2/block_devices/", "/dev/");
        DUrl url = DUrl::fromDeviceId(devs);
        DFMSideBarItem *item = group->findItem(url);
        if (item) {
            q->removeItem(item);
        }
    });

    // Device/volume get renamed.
    q->connect(devices_watcher, &DAbstractFileWatcher::fileMoved, group, [group, q](const DUrl & fromUrl, const DUrl & toUrl) {
        Q_UNUSED(fromUrl);
        QUrlQuery query(toUrl);
        QString newName = query.queryItemValue("new_name");
        DUrl url = toUrl.adjusted(DUrl::RemoveQuery);
        DFMSideBarItem *item = group->findItem(url);
        // item could be NULL for a labelled optical media
        if (!item) {
            return;
        }
        DAbstractFileInfoPointer fileInfo = DFileService::instance()->createFileInfo(q, url);
        if (fileInfo) {
            if (!newName.isEmpty()) {
                item->setText(newName);
            } else {
                item->setText(fileInfo->fileDisplayName());
            }
        }
    });
}

void DFMSideBarPrivate::initUserShareItem()
{
    Q_Q(DFMSideBar);

    using DFM_STD_LOCATION = DFMStandardPaths::StandardLocation;
    DFMSideBarItemGroup *group = groupNameMap[q->groupName(DFMSideBar::GroupName::Network)];

    int count = fileService->getChildren(group, DUrl::fromUserShareFile("/"),
                                         QStringList(), QDir::AllEntries).count();
    if (count) {
        group->appendItem(new DFMSideBarNetworkItem(DFM_STD_LOCATION::UserShareRootPath));
    }

    userShareFileWatcher = fileService->createFileWatcher(q, DUrl::fromUserShareFile("/"), q);
    Q_CHECK_PTR(userShareFileWatcher);
    userShareFileWatcher->startWatcher();

    auto userShareLambda = [ = ]() {
        int cnt = fileService->getChildren(group, DUrl::fromUserShareFile("/"),
                                           QStringList(), QDir::AllEntries).count();
        DFMSideBarItem *item = group->findItem(DUrl::fromUserShareFile("/"));
        if (cnt > 0 && item == nullptr) {
            item = new DFMSideBarNetworkItem(DFM_STD_LOCATION::UserShareRootPath);
            group->appendItem(item);
        } else if (cnt == 0 && item) {
            q->removeItem(item);
            item = nullptr;
        }

        return item;
    };

    q->connect(userShareFileWatcher, &DAbstractFileWatcher::fileDeleted, q, userShareLambda);
    q->connect(userShareFileWatcher, &DAbstractFileWatcher::subfileCreated, q, [ = ]() {
        DFMSideBarItem *item = userShareLambda();
        if (item) {
            item->playAnimation();
        }
    });
}

void DFMSideBarPrivate::initTagsConnection()
{
    Q_Q(DFMSideBar);

#ifdef DISABLE_TAG_SUPPORT
    return;
#endif

    DFMSideBarItemGroup *group = groupNameMap[q->groupName(DFMSideBar::GroupName::Tag)];
    Q_CHECK_PTR(group);

    DAbstractFileWatcher *tags_watcher = fileService->createFileWatcher(q_func(), DUrl(TAG_ROOT), group);
    Q_CHECK_PTR(tags_watcher);
    tags_watcher->startWatcher();

    // New tag added.
    q->connect(tags_watcher, &DAbstractFileWatcher::subfileCreated, group, [group](const DUrl & url) {
        group->appendItem(new DFMSideBarTagItem(url));
        group->saveItemOrder();
    });

    // Tag get removed.
    q->connect(tags_watcher, &DAbstractFileWatcher::fileDeleted, group, [group, q](const DUrl & url) {
        DFMSideBarItem *item = group->findItem(url);
        Q_CHECK_PTR(item); // should always find one
        q->removeItem(item);
        group->saveItemOrder();
    });

    // Tag got rename
    q->connect(tags_watcher, &DAbstractFileWatcher::fileMoved, group,
    [this, group, q](const DUrl & source, const DUrl & target) {
        DFMSideBarItem *item = q->itemAt(source);
        if (item) {
            item->setUrl(target);
            group->saveItemOrder();
        }
    });

    // Tag changed color
    q->connect(tags_watcher, &DAbstractFileWatcher::fileAttributeChanged, group, [group](const DUrl & url) {
        DFMSideBarItem *item = group->findItem(url);
        item->setIconFromThemeConfig("BookmarkItem." + TagManager::instance()->getTagColorName(url.tagName()));
    });
}

void DFMSideBarPrivate::initRecentConnection()
{
    Q_Q(DFMSideBar);

    q->connect(DFMApplication::instance(), &DFMApplication::recentDisplayChanged, q, [=] (bool enable) {
        DFMSideBarItemGroup *group = groupNameMap[q->groupName(DFMSideBar::GroupName::Common)];
        DFMSideBarItem *item = group->findItem(DUrl(RECENT_ROOT));

        if (enable && !item) {
            group->insertItem(0, new DFMSideBarRecentItem(DFMStandardPaths::StandardLocation::RecentPath));
        } else if (item && !enable) {
            DAbstractFileWatcher::ghostSignal(DUrl(RECENT_ROOT), &DAbstractFileWatcher::fileDeleted, DUrl(RECENT_ROOT));
            q->removeItem(item);
        }
    });
}

void DFMSideBarPrivate::setGroupSaveItemOrder(DFMSideBarItemGroup *group, DFMSideBar::GroupName groupType)
{
    switch (groupType) {
    case DFMSideBar::GroupName::Bookmark:
    case DFMSideBar::GroupName::Tag:
        group->setSaveItemOrder(true);
        break;
    default:
        group->setSaveItemOrder(false);
        break;
    }
}

void DFMSideBarPrivate::addItemToGroup(DFMSideBarItemGroup *group, DFMSideBar::GroupName groupType)
{
    Q_Q(DFMSideBar);

    // to make group touch less DFM internal implement, we add item here.
    using DFM_STD_LOCATION = DFMStandardPaths::StandardLocation;

    switch (groupType) {
    case DFMSideBar::GroupName::Common:
        if (DFMApplication::instance()->genericAttribute(DFMApplication::GA_ShowRecentFileEntry).toBool()) {
            group->appendItem(new DFMSideBarRecentItem(DFM_STD_LOCATION::RecentPath));
        }
        group->appendItem(new DFMSideBarHomeItem());
        group->appendItem(new DFMSideBarDefaultItem(DFM_STD_LOCATION::DesktopPath));
        group->appendItem(new DFMSideBarDefaultItem(DFM_STD_LOCATION::VideosPath));
        group->appendItem(new DFMSideBarDefaultItem(DFM_STD_LOCATION::MusicPath));
        group->appendItem(new DFMSideBarDefaultItem(DFM_STD_LOCATION::PicturesPath));
        group->appendItem(new DFMSideBarDefaultItem(DFM_STD_LOCATION::DocumentsPath));
        group->appendItem(new DFMSideBarDefaultItem(DFM_STD_LOCATION::DownloadsPath));
        group->appendItem(new DFMSideBarTrashItem());
        break;
    case DFMSideBar::GroupName::Device:
        group->appendItem(new DFMSideBarDefaultItem(DFM_STD_LOCATION::ComputerRootPath));

        if (!DFMApplication::instance()->genericAttribute(DFMApplication::GA_HiddenSystemPartition).toBool()) {
            group->appendItem(new DFMSideBarRootItem());
        }
        break;
    case DFMSideBar::GroupName::Bookmark: {
        QList<DAbstractFileInfoPointer> bookmark_infos = DFileService::instance()->getChildren(q_func(), DUrl(BOOKMARK_ROOT),
                                                         QStringList(), QDir::AllEntries);
        QList<DFMSideBarItem *> unsortedList;
        for (const DAbstractFileInfoPointer &info : bookmark_infos) {
            unsortedList << new DFMSideBarBookmarkItem(info->fileUrl());
        }
        appendListWithOrder(unsortedList, group);
        break;
    }
    case DFMSideBar::GroupName::Network:
        group->appendItem(new DFMSideBarNetworkItem(DFM_STD_LOCATION::NetworkRootPath));
        break;
    case DFMSideBar::GroupName::Tag: {
        auto tag_infos = DFileService::instance()->getChildren(q_func(), DUrl(TAG_ROOT),
                              QStringList(), QDir::AllEntries);
        QList<DFMSideBarItem *> unsortedList;
        for (const DAbstractFileInfoPointer &info : tag_infos) {
            unsortedList << new DFMSideBarTagItem(info->fileUrl());
        }
        appendListWithOrder(unsortedList, group);
        break;
    }
    default: // make compiler happy
        break;
    }
}

/*!
 * \class DFMSideBar
 * \inmodule dde-file-manager-lib
 *
 * \brief DFMSideBar is the left sidebar widget of Deepin File Manager
 *
 * DFMSideBar is the left sidebar widget of Deepin File Manager, provide the interface to manage
 * left sidebar state. Sidebar holds some DFMSideBarItemGroup s which hold their DFMSideBarItem s,
 * and DFMSideBar provide interface to access and manage sidebar items without touching the
 * DFMSideBarItemGroup internal class.
 *
 * \sa DFMSideBarItem
 */

DFMSideBar::DFMSideBar(QWidget *parent)
    : QScrollArea(parent)
    , d_ptr(new DFMSideBarPrivate(this))
{
    DThemeManager::instance()->registerWidget(this);
    QScroller::grabGesture(this);
}

DFMSideBar::~DFMSideBar()
{

}

bool DFMSideBar::contextMenuEnabled() const
{
    Q_D(const DFMSideBar);

    return d->contextMenuEnabled;
}

int DFMSideBar::count() const
{
    Q_D(const DFMSideBar);

    int count = 0;

    QMap<QString, DFMSideBarItemGroup *>::const_iterator i;
    for (i = d->groupNameMap.begin(); i != d->groupNameMap.end(); ++i) {
        count += i.value()->itemCount();
    }

    return count;
}

QStringList DFMSideBar::groupList() const
{
    Q_D(const DFMSideBar);

    QStringList groupNameList;

    QMap<QString, DFMSideBarItemGroup *>::const_iterator i;
    for (i = d->groupNameMap.begin(); i != d->groupNameMap.end(); ++i) {
        groupNameList.append(i.key());
    }

    return groupNameList;
}

/*!
 * \brief Set current sidebar state by current \a url.
 *
 * \param url Current url
 *
 * Will set the checked(highlighted) sidebar item by the given \a url.
 */
void DFMSideBar::setCurrentUrl(const DUrl &url)
{
    Q_D(DFMSideBar);

    if (d->lastCheckedItem) {
        d->lastCheckedItem->setChecked(false);
    }

    DFMSideBarItem *item = itemAt(url);

    if (item) {
        d->lastCheckedItem = item;
        d->lastCheckedItem->setChecked(true);
    }
}

/*!
 * \brief Should we display sidebar items' context menu?
 */
void DFMSideBar::setContextMenuEnabled(bool enable)
{
    Q_D(DFMSideBar);

    d->contextMenuEnabled = enable;
}

/*!
 * \brief Hide sidebar items by given url \a schemes .
 *
 * Notice that this is for *HIDE* the items, NOT for display a *DISABLED* state.
 */
void DFMSideBar::setDisableUrlSchemes(const QSet<QString> &schemes)
{
    Q_D(DFMSideBar);

    d->disabledSchemes = schemes;

    for (QString &key : d->groupNameMap.keys()) {
        DFMSideBarItemGroup *groupPointer = d->groupNameMap.value(key);
        groupPointer->setDisableUrlSchemes(schemes);
    }
}

/*!
 * \brief Let group can save item order to config file
 *
 * \param groupName The group name.
 *
 * \param saveItemOrder Save or not.
 *
 * Notice that it will do nothing if group not exist, and you should also call appendListWithOrder
 * if you need to use the saved order.
 *
 * \sa appendListWithOrder
 */
void DFMSideBar::setGroupSaveItemOrder(bool saveItemOrder, const QString &group)
{
    Q_D(DFMSideBar);

    if (d->groupNameMap.contains(group)) {
        DFMSideBarItemGroup *groupPointer = d->groupNameMap[group];
        groupPointer->setSaveItemOrder(saveItemOrder);
    }
}

/*!
 * \brief Return a url scheme list of the hidden sidebar items
 * \return
 */
QSet<QString> DFMSideBar::disableUrlSchemes() const
{
    Q_D(const DFMSideBar);

    return d->disabledSchemes;
}

/*!
 * \brief Add a sidebar item into a sidebar group.
 * \param item The item which is going to be added.
 * \param group The name (in `QString`, all lower case) of the sidebar group.
 * \return the index of the inserted item in that group.
 */
int DFMSideBar::addItem(DFMSideBarItem *item, const QString &group)
{
    Q_D(DFMSideBar);

    int index;

    if (d->groupNameMap.contains(group)) {
        DFMSideBarItemGroup *groupPointer = d->groupNameMap[group];
        index = groupPointer->appendItem(item);
    } else {
        QString groupName = group.isEmpty() ? "" : group;
        DFMSideBarItemGroup *group = new DFMSideBarItemGroup(groupName);
        d->groupConnectionRegister(group);
        d->groupNameMap[groupName] = group;
        index = group->appendItem(item);
        d->mainLayout->addLayout(group);
    }

    if (d->disabledSchemes.contains(item->url().scheme())) {
        item->hide();
    }

    return index;
}

/*!
 * \fn void DFMSideBar::insertItem(int index, DFMSideBarItem *item, const QString &groupName)
 * \brief Insert \a item before \a index into \a groupName .
 *
 * Insert a DFMSideBarItem \a item to group before \a index into the
 * given \a groupName .
 */
void DFMSideBar::insertItem(int index, DFMSideBarItem *item, const QString &group)
{
    Q_D(DFMSideBar);

    if (d->groupNameMap.contains(group)) {
        DFMSideBarItemGroup *groupPointer = d->groupNameMap[group];
        groupPointer->insertItem(index, item);
    }

    if (d->disabledSchemes.contains(item->url().scheme())) {
        item->hide();
    }
}

void DFMSideBar::removeItem(int index, const QString &group)
{
    Q_D(DFMSideBar);

    if (d->groupNameMap.contains(group)) {
        DFMSideBarItemGroup *groupPointer = d->groupNameMap[group];
        Q_CHECK_PTR(groupPointer);
        if ((*groupPointer)[index] == d->lastCheckedItem) {
            d->lastCheckedItem = nullptr;
        }
        groupPointer->removeItem(index);
    }
}

void DFMSideBar::removeItem(DFMSideBarItem *item)
{
    Q_D(DFMSideBar);

    if (item == d->lastCheckedItem) {
        d->lastCheckedItem = nullptr;
    }

    DFMSideBarItemGroup *groupPointer = d->groupNameMap[item->groupName()];
    Q_CHECK_PTR(groupPointer);
    int index = groupPointer->itemIndex(item);
    if (index != -1) {
        groupPointer->removeItem(index);
        return;
    }
}

/*!
 * \brief Get the item index from its group.
 *
 * \return the index from its group. return -1 if not found the item.
 *
 * Please notice that this return the index from the item's owner group,
 * not the index of the full sidebar items.
 */
int DFMSideBar::itemIndex(const DFMSideBarItem *item) const
{
    Q_D(const DFMSideBar);

    DFMSideBarItemGroup *groupPointer = d->groupNameMap[item->groupName()];
    Q_CHECK_PTR(groupPointer);
    return groupPointer->itemIndex(item);
}

/*!
 * \brief Insert a list of url to a group with saved sort order support.
 *
 * It ok if a group is not set setGroupSaveItemOrder as true.
 *
 * \sa setGroupSaveItemOrder
 */
void DFMSideBar::appendListWithOrder(QList<DFMSideBarItem *> itemList, const QString &group) const
{
    Q_D(const DFMSideBar);

    DFMSideBarItemGroup *groupPointer = d->groupNameMap[group];

    if (groupPointer) {
        d->appendListWithOrder(itemList, groupPointer);
    }
}

/*!
 * \fn DFMSideBarItem *DFMSideBar::itemAt(int index, const QString &groupName) const
 *
 * \brief Get the reference of DFMSideBarItem at \a index in the given \a groupName.
 * \param index The item index which is wanted.
 * \param groupName The group name which is wanted.
 * \return the reference (pointer) of the item.
 */
DFMSideBarItem *DFMSideBar::itemAt(int index, const QString &group) const
{
    Q_D(const DFMSideBar);

    if (d->groupNameMap.contains(group)) {
        DFMSideBarItemGroup *groupPointer = d->groupNameMap[group];
        return (*groupPointer)[index];
    }

    return nullptr;
}

DFMSideBarItem *DFMSideBar::itemAt(const DUrl &url) const
{
    Q_D(const DFMSideBar);

    for (QString &key : d->groupNameMap.keys()) {
        DFMSideBarItemGroup *groupPointer = d->groupNameMap.value(key);
        DFMSideBarItem *item = groupPointer->findItem(url);

        if (item) {
            return item;
        }
    }

    // fallback
    for (QString &key : d->groupNameMap.keys()) {
        DFMSideBarItemGroup *groupPointer = d->groupNameMap.value(key);

        for (int i = 0; i < groupPointer->itemCount(); ++i) {
            DFMSideBarItem *item = (*groupPointer)[i];

            DUrl redirectedFileUrl = item->url();

            forever {
                if (redirectedFileUrl == url) {
                    return item;
                }

                const DAbstractFileInfoPointer &info = fileService->createFileInfo(this, redirectedFileUrl);

                if (info && info->canRedirectionFileUrl()) {
                    redirectedFileUrl = info->redirectedFileUrl();
                } else {
                    break;
                }
            }
        }
    }

    return nullptr;
}

DFMSideBarItem *DFMSideBar::takeItem(int index, const QString &group)
{
    Q_D(DFMSideBar);

    if (d->groupNameMap.contains(group)) {
        DFMSideBarItemGroup *groupPointer = d->groupNameMap[group];
        DFMSideBarItem *item = groupPointer->takeItem(index);
        if (d->lastCheckedItem == item) {
            d->lastCheckedItem = nullptr;
        }
        return item;
    }

    return nullptr;
}

int DFMSideBar::itemCount(const QString &group) const
{
    Q_D(const DFMSideBar);

    if (d->groupNameMap.contains(group)) {
        DFMSideBarItemGroup *groupPointer = d->groupNameMap[group];
        return groupPointer->itemCount();
    }

    return 0;
}

QRect DFMSideBar::groupGeometry(const QString &group) const
{
    Q_D(const DFMSideBar);

    if (d->groupNameMap.contains(group)) {
        DFMSideBarItemGroup *groupPointer = d->groupNameMap[group];
        return groupPointer->geometry();
    }

    // or throw a exception if not found?
    return QRect();
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

void DFMSideBar::resizeEvent(QResizeEvent *event)
{
    Q_D(DFMSideBar);

    d->mainLayoutHolder->resize(width(), d->mainLayoutHolder->height());

    return QScrollArea::resizeEvent(event);
}

DFM_END_NAMESPACE

#include "moc_dfmsidebar.cpp"
