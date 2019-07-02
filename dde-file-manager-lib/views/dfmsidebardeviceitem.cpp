/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
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
#include "dfilemanagerwindow.h"
#include "dfmsidebardeviceitem.h"

#include "singleton.h"

#include "app/define.h"
#include "app/filesignalmanager.h"
#include "views/windowmanager.h"
#include "gvfs/gvfsmountmanager.h"
#include "dblockdevice.h"
#include "ddiskdevice.h"
#include "ddiskmanager.h"

#include <QMenu>
#include <QAction>

DFM_BEGIN_NAMESPACE

DFMSideBarDeviceItem::DFMSideBarDeviceItem(DUrl url, QWidget *parent)
    : DFMSideBarItem(url, parent)
{
    const DAbstractFileInfoPointer infoPointer = fileService->createFileInfo(this, url);
    QVariantHash info = infoPointer->extraProperties();
    setText(infoPointer->fileDisplayName());
    setAutoOpenUrlOnClick(false);

    connect(this, &DFMSideBarDeviceItem::clicked,
            this, &DFMSideBarDeviceItem::itemOnClick);

    unmountButton = new DImageButton(this);
    unmountButton->setVisible(info.value("canUnmount", false).toBool());
    this->setContentWidget(unmountButton);

    connect(unmountButton, &DImageButton::clicked,
            this, &DFMSideBarDeviceItem::doUnmountOrEject);

    switch (info.value("mediaType", 0).toInt()) {
    case UDiskDeviceInfo::MediaType::native:
        setIconFromThemeConfig("BookmarkItem.Disk");
        break;
    case UDiskDeviceInfo::MediaType::removable:
        setIconFromThemeConfig("BookmarkItem.Usb");
        break;
    case UDiskDeviceInfo::MediaType::dvd:
        setIconFromThemeConfig("BookmarkItem.Dvd");
        break;
    case UDiskDeviceInfo::MediaType::phone:
        setIconFromThemeConfig("BookmarkItem.Android");
        break;
    case UDiskDeviceInfo::MediaType::iphone:
        setIconFromThemeConfig("BookmarkItem.Iphone");
        break;
    default:
        break;
    }
}

QVariantHash DFMSideBarDeviceItem::getExtraProperties() const
{
    return fileService->createFileInfo(this, url())->extraProperties();
}

QMenu *DFMSideBarDeviceItem::createStandardContextMenu() const
{
    QMenu *menu = new QMenu();
    const DAbstractFileInfoPointer infoPointer = fileService->createFileInfo(this, url());
    DFileManagerWindow *wnd = qobject_cast<DFileManagerWindow *>(topLevelWidget());
    bool shouldDisable = !WindowManager::tabAddableByWinId(wnd->windowId());
    QVariantHash info = getExtraProperties();
    DUrl deviceIdUrl;

    deviceIdUrl.setQuery(info.value("deviceId").toString());

    menu->addAction(QObject::tr("Open in new window"), [this, info, deviceIdUrl]() {
        if (info.value("isMounted", false).toBool()) {
            WindowManager::instance()->showNewWindow(url(), true);
        } else {
            appController->actionOpenDiskInNewWindow(dMakeEventPointer<DFMUrlBaseEvent>(this, deviceIdUrl));
        }
    });

    menu->addAction(QObject::tr("Open in new tab"), [wnd, this, info, deviceIdUrl]() {
        if (info.value("isMounted", false).toBool()) {
            wnd->openNewTab(url());
        } else {
            appController->actionOpenDiskInNewTab(dMakeEventPointer<DFMUrlBaseEvent>(this, deviceIdUrl));
        }
    })->setDisabled(shouldDisable);

    menu->addSeparator();

    if (infoPointer->canRename()) {
        menu->addAction(QObject::tr("Rename"), [this]() {
            DFMSideBarDeviceItem *ccItem = const_cast<DFMSideBarDeviceItem *>(this);
            ccItem->showRenameEditor();
        });
    }

//    if (info.value("canEject", false).toBool()) {
//        menu->addAction(QObject::tr("Eject"), [this, info]() {
//            gvfsMountManager->eject(info.value("deviceId").toString());
//        });
//    }

    if (info.value("canStop", false).toBool()) {
        menu->addAction(QObject::tr("Safely Remove"), [this, info, deviceIdUrl]() {
            AppController::instance()->actionSafelyRemoveDrive(dMakeEventPointer<DFMUrlBaseEvent>(this, deviceIdUrl));
        });
    }

    if (info.value("canMount", false).toBool() && !info.value("isMounted", false).toBool()) {
        menu->addAction(QObject::tr("Mount"), [info]() {
            gvfsMountManager->mount(info.value("deviceId").toString());
        });
    }

//    if (info.value("canUnmount", false).toBool()) {
//        menu->addAction(QObject::tr("Unmount"), [this, info]() {
//            gvfsMountManager->unmount(info.value("deviceId").toString());
//        });
//    }

    // According to the designer and PM, we should use "Unmount" even we are actually doing *Eject* for removable device.
    // This behavior should be discussed later.
    if (info.value("canUnmount", false).toBool()) {
        menu->addAction(QObject::tr("Unmount"), this, SLOT(doUnmountOrEject()));
    }

    if (info.value("mediaType", 0).toInt() == UDiskDeviceInfo::MediaType::removable && !info.value("optical", false).toBool()) {
        menu->addAction(QObject::tr("Format"), [this, info, deviceIdUrl]() {
            AppController::instance()->actionFormatDevice(dMakeEventPointer<DFMUrlBaseEvent>(this, deviceIdUrl));
        });
    }

    if (info.value("opticalReuseable", false).toBool()) {
        menu->addAction(QObject::tr("Erase"), [this, info, deviceIdUrl]() {
            AppController::instance()->actionOpticalBlank(dMakeEventPointer<DFMUrlBaseEvent>(this, deviceIdUrl));
        });
    }

    // Device can be a network scheme, like smb://, ftp:// and sftp://
    QString devicePathScheme = DUrl::fromUserInput(info.value("deviceId").toString()).scheme();
    if (devicePathScheme == SMB_SCHEME || devicePathScheme == FTP_SCHEME || devicePathScheme == SFTP_SCHEME || devicePathScheme == DAV_SCHEME) {
        menu->addAction(QObject::tr("Log out and unmount"), [this, deviceIdUrl]() {
            AppController::instance()->actionForgetPassword(dMakeEventPointer<DFMUrlBaseEvent>(this, deviceIdUrl));
        });
    }

    menu->addSeparator();

    QAction *propertyAction = new QAction(QObject::tr("Disk info"), menu);
    connect(propertyAction, &QAction::triggered, this, [this, info]() {
        DUrl mountPointUrl(info.value("mountPointUrl", QString()).toString());
        mountPointUrl = mountPointUrl.isEmpty() ? url() : mountPointUrl;
        mountPointUrl.setQuery(info.value("deviceId", "what?").toString());
        fileSignalManager->requestShowPropertyDialog(DFMUrlListBaseEvent(this, {mountPointUrl}));
    });
    propertyAction->setDisabled(!info.value("isMounted", false).toBool());
    menu->addAction(propertyAction);

    return menu;
}

void DFMSideBarDeviceItem::itemOnClick()
{
    QVariantHash info = getExtraProperties();

    if (info.value("isMounted", false).toBool()) {
        DFileManagerWindow *wnd = qobject_cast<DFileManagerWindow *>(topLevelWidget());
        wnd->cd(url());
    } else if (info.value("canMount", false).toBool()) {
        DUrl newUrl;
        newUrl.setQuery(info.value("deviceId").toString());
        appController->actionOpenDisk(dMakeEventPointer<DFMUrlBaseEvent>(this, newUrl));
    }
}

void DFMSideBarDeviceItem::doUnmountOrEject()
{
    QVariantHash info = getExtraProperties();

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
        AppController::instance()->actionSafelyRemoveDrive(dMakeEventPointer<DFMUrlBaseEvent>(this, deviceIdUrl));
    }
}

DFM_END_NAMESPACE
