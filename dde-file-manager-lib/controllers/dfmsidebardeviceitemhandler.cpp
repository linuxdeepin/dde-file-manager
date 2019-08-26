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

#include "dfmsidebardeviceitemhandler.h"

#include "singleton.h"
#include "dfmevent.h"
#include "app/define.h"
#include "app/filesignalmanager.h"
#include "dfileservices.h"
#include "views/dfilemanagerwindow.h"
#include "views/dfmsidebar.h"
#include "views/windowmanager.h"
#include "gvfs/gvfsmountmanager.h"
#include "interfaces/dfmsidebaritem.h"
#include "deviceinfo/udiskdeviceinfo.h"
#include "dfmsidebarmanager.h"

#include <QAction>

DFM_BEGIN_NAMESPACE

DViewItemAction *DFMSideBarDeviceItemHandler::createUnmountOrEjectAction(const DUrl &url, bool withText)
{
    DViewItemAction * action = new DViewItemAction(Qt::AlignCenter, QSize(16, 16), QSize(), true);
    if (withText) {
        action->setText(QObject::tr("Unmount"));
    }
    action->setIcon(QIcon::fromTheme("media-eject-symbolic"));

    QObject::connect(action, &QAction::triggered, action, [url](){
        const DAbstractFileInfoPointer infoPointer = DFileService::instance()->createFileInfo(nullptr, url);
        QVariantHash info = infoPointer->extraProperties();
        if (info.value("isRemovable", false).toBool() && info.value("canEject", false).toBool()) {
            gvfsMountManager->eject(info.value("deviceId").toString());
            return;
        }

        if (info.value("canUnmount", false).toBool()) {
            gvfsMountManager->unmount(info.value("deviceId").toString());
        }

        if (info.value("canStop", false).toBool()) {
            DUrl deviceIdUrl;
            deviceIdUrl.setQuery(info.value("deviceId").toString());
            AppController::instance()->actionSafelyRemoveDrive(dMakeEventPointer<DFMUrlBaseEvent>(nullptr, deviceIdUrl));
        }
    });

    return action;
}

DFMSideBarItem *DFMSideBarDeviceItemHandler::createItem(const DUrl &url)
{
    const DAbstractFileInfoPointer infoPointer = DFileService::instance()->createFileInfo(nullptr, url);
    QVariantHash info = infoPointer->extraProperties();
    QString displayName = infoPointer->fileDisplayName();
    QString iconName;

    switch (info.value("mediaType", 0).toInt()) {
    case UDiskDeviceInfo::MediaType::native:
        iconName = "drive-harddisk-symbolic";
        break;
    case UDiskDeviceInfo::MediaType::removable:
        iconName = "drive-removable-media-symbolic";
        break;
    case UDiskDeviceInfo::MediaType::dvd:
        iconName = "media-optical-symbolic";
        break;
    case UDiskDeviceInfo::MediaType::phone:
        iconName = "phone-symbolic";
        break;
    case UDiskDeviceInfo::MediaType::iphone:
        iconName = "phone-apple-iphone-symbolic";
        break;
    default:
        iconName = "drive-harddisk-symbolic";
        break;
    }

    DFMSideBarItem * item = new DFMSideBarItem(QIcon::fromTheme(iconName), displayName, url);
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren);
    item->setData(SIDEBAR_ID_DEVICE, DFMSideBarItem::ItemUseRegisteredHandlerRole);
    DViewItemActionList lst;
    DViewItemAction * act = createUnmountOrEjectAction(url, false);
    act->setIcon(QIcon::fromTheme("media-eject-symbolic"));
    act->setVisible(info.value("canUnmount", false).toBool());
    lst.push_back(act);
    item->setActionList(Qt::RightEdge, lst);
    return item;
}

DFMSideBarDeviceItemHandler::DFMSideBarDeviceItemHandler(QObject *parent)
    : DFMSideBarItemInterface (parent)
{

}

void DFMSideBarDeviceItemHandler::cdAction(const DFMSideBar *sidebar, const DFMSideBarItem *item)
{
    QVariantHash info = DFileService::instance()->createFileInfo(this, item->url())->extraProperties();

    if (info.value("isMounted", false).toBool()) {
        DFileManagerWindow *wnd = qobject_cast<DFileManagerWindow *>(sidebar->topLevelWidget());
        wnd->cd(item->url());
    } else if (info.value("canMount", false).toBool()) {
        DUrl newUrl;
        newUrl.setQuery(info.value("deviceId").toString());
        AppController::instance()->actionOpenDisk(dMakeEventPointer<DFMUrlBaseEvent>(this, newUrl));
    }
}

QMenu *DFMSideBarDeviceItemHandler::contextMenu(const DFMSideBar *sidebar, const DFMSideBarItem *item)
{
    QMenu *menu = new QMenu();
    const DAbstractFileInfoPointer infoPointer = DFileService::instance()->createFileInfo(this, item->url());
    DFileManagerWindow *wnd = qobject_cast<DFileManagerWindow *>(sidebar->topLevelWidget());
    bool shouldDisable = !WindowManager::tabAddableByWinId(wnd->windowId());
    QVariantHash info = infoPointer->extraProperties();
    DUrl deviceIdUrl;

    deviceIdUrl.setQuery(info.value("deviceId").toString());

    menu->addAction(QObject::tr("Open in new window"), [item, info, deviceIdUrl]() {
        if (info.value("isMounted", false).toBool()) {
            WindowManager::instance()->showNewWindow(item->url(), true);
        } else {
            AppController::instance()->actionOpenDiskInNewWindow(dMakeEventPointer<DFMUrlBaseEvent>(nullptr, deviceIdUrl));
        }
    });

    menu->addAction(QObject::tr("Open in new tab"), [wnd, item, info, deviceIdUrl]() {
        if (info.value("isMounted", false).toBool()) {
            wnd->openNewTab(item->url());
        } else {
            AppController::instance()->actionOpenDiskInNewTab(dMakeEventPointer<DFMUrlBaseEvent>(nullptr, deviceIdUrl));
        }
    })->setDisabled(shouldDisable);

    menu->addSeparator();

    if (infoPointer->canRename()) {
        menu->addAction(QObject::tr("Rename"), [sidebar, item]() {
            int index = sidebar->findItem(item);
            if (index >= 0) {
                sidebar->openItemEditor(index);
            }
        });
    }

    if (info.value("canStop", false).toBool()) {
        menu->addAction(QObject::tr("Safely Remove"), [info, deviceIdUrl]() {
            AppController::instance()->actionSafelyRemoveDrive(dMakeEventPointer<DFMUrlBaseEvent>(nullptr, deviceIdUrl));
        });
    }

    if (info.value("canMount", false).toBool() && !info.value("isMounted", false).toBool()) {
        menu->addAction(QObject::tr("Mount"), [info]() {
            gvfsMountManager->mount(info.value("deviceId").toString());
        });
    }

    // According to the designer and PM, we should use "Unmount" even we are actually doing *Eject* for removable device.
    // This behavior should be discussed later.
    if (info.value("canUnmount", false).toBool()) {
        menu->addAction(DFMSideBarDeviceItemHandler::createUnmountOrEjectAction(item->url(), true));
    }

    if (info.value("mediaType", 0).toInt() == UDiskDeviceInfo::MediaType::removable && !info.value("optical", false).toBool()) {
        menu->addAction(QObject::tr("Format"), [info, deviceIdUrl]() {
            AppController::instance()->actionFormatDevice(dMakeEventPointer<DFMUrlBaseEvent>(nullptr, deviceIdUrl));
        });
    }

    if (info.value("opticalReuseable", false).toBool()) {
        menu->addAction(QObject::tr("Erase"), [info, deviceIdUrl]() {
            AppController::instance()->actionOpticalBlank(dMakeEventPointer<DFMUrlBaseEvent>(nullptr, deviceIdUrl));
        });
    }

    // Device can be a network scheme, like smb://, ftp:// and sftp://
    QString devicePathScheme = DUrl::fromUserInput(info.value("deviceId").toString()).scheme();
    if (devicePathScheme == SMB_SCHEME || devicePathScheme == FTP_SCHEME || devicePathScheme == SFTP_SCHEME || devicePathScheme == DAV_SCHEME) {
        menu->addAction(QObject::tr("Log out and unmount"), [deviceIdUrl]() {
            AppController::instance()->actionForgetPassword(dMakeEventPointer<DFMUrlBaseEvent>(nullptr, deviceIdUrl));
        });
    }

    menu->addSeparator();

    menu->addAction(QObject::tr("Disk info"), [item, info]() {
        DUrl mountPointUrl(info.value("mountPointUrl", QString()).toString());
        mountPointUrl = mountPointUrl.isEmpty() ? item->url() : mountPointUrl;
        mountPointUrl.setQuery(info.value("deviceId", "what?").toString());
        fileSignalManager->requestShowPropertyDialog(DFMUrlListBaseEvent(nullptr, {mountPointUrl}));
    })->setDisabled(!info.value("isMounted", false).toBool());

    return menu;
}

DFM_END_NAMESPACE
