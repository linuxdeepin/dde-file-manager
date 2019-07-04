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
#include "dfmsidebaropticaldevitem.h"

#include "singleton.h"

#include "app/define.h"
#include "app/filesignalmanager.h"
#include "views/windowmanager.h"
#include "ddiskmanager.h"
#include "disomaster.h"

#include <QMenu>
#include <QAction>

DFM_BEGIN_NAMESPACE

DFMSideBarOpticalDevItem::DFMSideBarOpticalDevItem(DUrl url, QWidget *parent)
    : DFMSideBarItem(url, parent)
{
    QString udiskspath(url.path());
    udiskspath.replace("/dev/", "/org/freedesktop/UDisks2/block_devices/");
    blk.reset(DDiskManager::createBlockDevice(udiskspath));
    drv.reset(DDiskManager::createDiskDevice(blk->drive()));
    blk->setWatchChanges(true);
    reloadLabel();
    setIconFromThemeConfig("BookmarkItem.Dvd");
    setAutoOpenUrlOnClick(false);

    connect(this, &DFMSideBarOpticalDevItem::clicked,
            this, &DFMSideBarOpticalDevItem::itemOnClick);

    unmountButton = new DImageButton(this);
    unmountButton->setVisible(drv->mediaAvailable());
    this->setContentWidget(unmountButton);

    connect(unmountButton, &DImageButton::clicked,
            this, &DFMSideBarOpticalDevItem::eject);

    connect(blk.data(), &DBlockDevice::idLabelChanged, this, &DFMSideBarOpticalDevItem::reloadLabel);
}

int DFMSideBarOpticalDevItem::sortingPriority() const
{
    return 1;
}

QMenu *DFMSideBarOpticalDevItem::createStandardContextMenu() const
{
    QMenu *menu = new QMenu();
    DFileManagerWindow *wnd = qobject_cast<DFileManagerWindow *>(topLevelWidget());
    bool shouldDisable = !WindowManager::tabAddableByWinId(wnd->windowId());
    DUrl deviceIdUrl;

    deviceIdUrl.setQuery(blk->device());

    menu->addAction(QObject::tr("Open in new window"), [this, deviceIdUrl]() {
        if (blk->mountPoints().length() || drv->opticalBlank()) {
            WindowManager::instance()->showNewWindow(url(), true);
        } else {
            appController->actionOpenDiskInNewWindow(dMakeEventPointer<DFMUrlBaseEvent>(this, deviceIdUrl));
        }
    });

    menu->addAction(QObject::tr("Open in new tab"), [wnd, this, deviceIdUrl]() {
        if (blk->mountPoints().length() || drv->opticalBlank()) {
            wnd->openNewTab(url());
        } else {
            appController->actionOpenDiskInNewTab(dMakeEventPointer<DFMUrlBaseEvent>(this, deviceIdUrl));
        }
    })->setDisabled(shouldDisable);

    menu->addSeparator();

    if (drv->mediaAvailable() && !drv->opticalBlank() && blk->mountPoints().empty()) {
        menu->addAction(QObject::tr("Mount"), [this]() {
            blk->mount({});
        });
    }

    // According to the designer and PM, we should use "Unmount" even we are actually doing *Eject* for removable device.
    // This behavior should be discussed later.
    if (drv->mediaAvailable()) {
        menu->addAction(QObject::tr("Unmount"), this, &DFMSideBarOpticalDevItem::eject);
    }

    if (drv->media().contains("_rw")) {
        menu->addAction(QObject::tr("Erase"), [this, deviceIdUrl]() {
            AppController::instance()->actionOpticalBlank(dMakeEventPointer<DFMUrlBaseEvent>(this, deviceIdUrl));
        });
    }

    menu->addSeparator();

    QAction *propertyAction = new QAction(QObject::tr("Disk info"), menu);
    connect(propertyAction, &QAction::triggered, this, [this]() {
        DUrl mountPointUrl(blk->mountPoints().size() ? blk->mountPoints().front() : "");
        mountPointUrl = mountPointUrl.isEmpty() ? url() : mountPointUrl;
        mountPointUrl.setQuery(blk->device());
        fileSignalManager->requestShowPropertyDialog(DFMUrlListBaseEvent(this, {mountPointUrl}));
    });
    propertyAction->setDisabled(blk->mountPoints().size() == 0);
    menu->addAction(propertyAction);

    return menu;
}

void DFMSideBarOpticalDevItem::itemOnClick()
{
    if (ISOMaster->currentDevice() == url().path()) {
        return;
    }
    if (drv->mediaAvailable()) {
        DUrl newUrl;
        newUrl.setQuery(blk->device());
        if (blk->mountPoints().size() || drv->opticalBlank()) {
            DFileManagerWindow *wnd = qobject_cast<DFileManagerWindow *>(topLevelWidget());
            wnd->cd(url());
        } else {
            appController->actionOpenDisk(dMakeEventPointer<DFMUrlBaseEvent>(this, newUrl));
        }
    } else drv->eject({});
}

void DFMSideBarOpticalDevItem::eject()
{
    if (drv->ejectable()) {
        blk->unmount({});
        drv->eject({});
    }
}

void DFMSideBarOpticalDevItem::reloadLabel()
{
    QHash<QString, QString> mediamap = {
        {"optical_cd",             "CD-ROM"},
        {"optical_cd_r",           "CD-R"},
        {"optical_cd_rw",          "CD-RW"},
        {"optical_dvd",            "DVD-ROM"},
        {"optical_dvd_r",          "DVD-R"},
        {"optical_dvd_rw",         "DVD-RW"},
        {"optical_dvd_ram",        "DVD-RAM"},
        {"optical_dvd_plus_r",     "DVD+R"},
        {"optical_dvd_plus_rw",    "DVD+RW"},
        {"optical_dvd_plus_r_dl",  "DVD+R/DL"},
        {"optical_dvd_plus_rw_dl", "DVD+RW/DL"},
        {"optical_bd",             "Bluray"},
        {"optical_bd_r",           "Bluray-R"},
        {"optical_bd_rw",          "Bluray-RE"},
        {"optical_hddvd",          "HD-DVD"},
        {"optical_hddvd_r",        "HD-DVD-R"},
        {"optical_hddvd_rw",       "HD-DVD-RW"}
    };
    if (drv->mediaAvailable()) {
        if (blk->idLabel().length()) {
            setText(blk->idLabel());
        } else {
            if (drv->opticalBlank()) {
                setText(tr("Blank %1 disc").arg(mediamap[drv->media()]));
            } else {
                setText(tr("%1 disc").arg(mediamap[drv->media()]));
            }
        }
    } else {
        QStringList mc = drv->mediaCompatibility();
        while (mc.size() && (mc.back() == "optical_mrw" || mc.back() == "optical_mrw_w")) {
            mc.pop_back();
        }
        Q_ASSERT(!mc.empty());
        setText(tr("%1 drive").arg(mediamap[mc.back()]));
    }
}
DFM_END_NAMESPACE
