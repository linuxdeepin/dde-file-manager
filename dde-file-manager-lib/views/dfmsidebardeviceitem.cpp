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

#include <QMenu>
#include <QAction>

DFM_BEGIN_NAMESPACE

DFMSideBarDeviceItem::DFMSideBarDeviceItem(DUrl url, QWidget *parent)
    : DFMSideBarItem(url, parent)
{
    const DAbstractFileInfoPointer infoPointer = fileService->createFileInfo(this, url);
    QVariantHash info = infoPointer->extensionPropertys();
    setText(infoPointer->fileDisplayName());
    setAutoOpenUrlOnClick(false);

    connect(this, &DFMSideBarDeviceItem::clicked,
            this, &DFMSideBarDeviceItem::itemOnClick);

    unmountButton = new DImageButton(this);
    unmountButton->setVisible(info.value("canUnmount", false).toBool());
    this->setContentWidget(unmountButton);

    connect(unmountButton, &DImageButton::clicked,
            this, &DFMSideBarDeviceItem::doUnmount);

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

QVariantHash DFMSideBarDeviceItem::getExtensionPropertys() const
{
    return fileService->createFileInfo(this, url())->extensionPropertys();
}

QMenu *DFMSideBarDeviceItem::createStandardContextMenu() const
{
    QMenu *menu = new QMenu();
    DFileManagerWindow *wnd = qobject_cast<DFileManagerWindow *>(topLevelWidget());
    QVariantHash info = getExtensionPropertys();
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
    });

    menu->addSeparator();

    if (info.value("canEject", false).toBool()) {
        menu->addAction(QObject::tr("Eject"), [this, info]() {
            gvfsMountManager->eject(info.value("deviceId").toString());
        });
    }

    if (info.value("canStop", false).toBool()) {
        menu->addAction(QObject::tr("Safely Remove"), [this, info, deviceIdUrl]() {
            AppController::instance()->actionSafelyRemoveDrive(dMakeEventPointer<DFMUrlBaseEvent>(this, deviceIdUrl));
        });
    }

    if (info.value("canMount", false).toBool() && !info.value("isMounted", false).toBool()) {
        menu->addAction(QObject::tr("Mount"), [this, info]() {
            gvfsMountManager->mount(info.value("deviceId").toString());
        });
    }

    if (info.value("canUnmount", false).toBool()) {
        menu->addAction(QObject::tr("Unmount"), this, SLOT(doUnmount()));
    }

    if (info.value("mediaType", 0).toInt() == UDiskDeviceInfo::MediaType::removable) {
        menu->addAction(QObject::tr("Format"), [this, info, deviceIdUrl]() {
            AppController::instance()->actionFormatDevice(dMakeEventPointer<DFMUrlBaseEvent>(this, deviceIdUrl));
        });
    }

    menu->addSeparator();

    QAction *propertyAction = new QAction(QObject::tr("Properties"), menu);
    connect(propertyAction, &QAction::triggered, this, [this]() {
        DUrlList list;
        list.append(url());
        fileSignalManager->requestShowPropertyDialog(DFMUrlListBaseEvent(this, list));
    });
    propertyAction->setDisabled(!info.value("isMounted", false).toBool());
    menu->addAction(propertyAction);

    return menu;
}

void DFMSideBarDeviceItem::itemOnClick()
{
    QVariantHash info = getExtensionPropertys();

    if (info.value("canMount", false).toBool()) {
        if (info.value("isMounted", false).toBool()) {
            DFileManagerWindow *wnd = qobject_cast<DFileManagerWindow *>(topLevelWidget());
            wnd->cd(url());
        } else {
            DUrl newUrl;
            newUrl.setQuery(info.value("deviceId").toString());
            appController->actionOpenDisk(dMakeEventPointer<DFMUrlBaseEvent>(this, newUrl));
        }
    }
}

void DFMSideBarDeviceItem::doUnmount()
{
    QVariantHash info = getExtensionPropertys();
    if (info.value("canUnmount", false).toBool()) {
        gvfsMountManager->unmount(info.value("deviceId").toString());
    }
}

DFM_END_NAMESPACE
