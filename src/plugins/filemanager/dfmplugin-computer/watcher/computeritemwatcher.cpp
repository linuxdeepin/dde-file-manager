/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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
#include "computeritemwatcher.h"
#include "controller/computercontroller.h"
#include "utils/computerutils.h"

#include "dfm-base/dbusservice/global_server_defines.h"
#include "dfm-base/dbusservice/dbus_interface/devicemanagerdbus_interface.h"
#include "dfm-base/utils/devicemanager.h"
#include "dfm-base/file/entry/entryfileinfo.h"
#include "dfm-base/base/schemefactory.h"

#include "services/filemanager/sidebar/sidebar_defines.h"
#include "services/filemanager/sidebar/sidebarservice.h"

#include <QDebug>
#include <QApplication>
#include <QWindow>

static DSB_FM_NAMESPACE::SideBarService *sbIns()
{
    auto &ctx = dpfInstance.serviceContext();
    static std::once_flag onceFlag;
    std::call_once(onceFlag, [&ctx]() {
        if (!ctx.load(DSB_FM_NAMESPACE::SideBarService::name()))
            abort();
    });

    return ctx.service<DSB_FM_NAMESPACE::SideBarService>(DSB_FM_NAMESPACE::SideBarService::name());
}

DFMBASE_USE_NAMESPACE
DPCOMPUTER_BEGIN_NAMESPACE

/*!
 * \class ComputerItemWatcher
 * \brief watches the change of computer item
 */
ComputerItemWatcher *ComputerItemWatcher::instance()
{
    static ComputerItemWatcher watcher;
    return &watcher;
}

ComputerItemWatcher::ComputerItemWatcher(QObject *parent)
    : QObject(parent)
{
    initConn();
}

ComputerItemWatcher::~ComputerItemWatcher()
{
}

ComputerDataList ComputerItemWatcher::items()
{
    ComputerDataList ret;

    ret.append(getUserDirItems());

    // these are all in Disk group
    bool hasInsertNewDisk = false;
    ret.push_back(getGroup(kGroupDisks));

    ret.append(getBlockDeviceItems(hasInsertNewDisk));
    ret.append(getProtocolDeviceItems(hasInsertNewDisk));
    // get stashed mounts
    //    ret.append(getStashedProtocolItems(hasInsertNewDisk));
    // get application entries
    //    ret.append(getAppEntryItems(hasInsertNewDisk));

    if (!hasInsertNewDisk)
        ret.pop_back();

    return ret;
}

bool ComputerItemWatcher::typeCompare(const ComputerItemData &a, const ComputerItemData &b)
{
    if (a.info && b.info) {
        if (a.info->order() == b.info->order())   // then sort by name
            return a.info->displayName() < b.info->displayName();
        else
            return a.info->order() < b.info->order();
    }
    return false;
}

void ComputerItemWatcher::initConn()
{
    connect(this, &ComputerItemWatcher::itemRemoved, this, &ComputerItemWatcher::removeSidebarItem);
    connect(DeviceManagerInstance.getDeviceInterface(), &DeviceManagerInterface::BlockDeviceAdded, this, &ComputerItemWatcher::onDeviceAdded);
    connect(DeviceManagerInstance.getDeviceInterface(), &DeviceManagerInterface::BlockDeviceRemoved, this, [this](const QString &id) {
        auto &&devUrl = ComputerUtils::makeBlockDevUrl(id);
        Q_EMIT this->itemRemoved(devUrl);
    });

    auto updateItem = [this](const QString &id) {
        auto &&devUrl = ComputerUtils::makeBlockDevUrl(id);
        Q_EMIT this->itemUpdated(devUrl);
    };
    connect(DeviceManagerInstance.getDeviceInterface(), &DeviceManagerInterface::BlockDeviceMounted, this, [updateItem](const QString &id) {
        auto datas = DeviceManagerInstance.invokeQueryBlockDeviceInfo(id);
        auto shellDevId = datas.value(GlobalServerDefines::DeviceProperty::kCryptoBackingDevice).toString();
        updateItem(shellDevId.length() > 1 ? shellDevId : id);
    });
    connect(DeviceManagerInstance.getDeviceInterface(), &DeviceManagerInterface::BlockDeviceUnmounted, this, updateItem);
    connect(DeviceManagerInstance.getDeviceInterface(), &DeviceManagerInterface::BlockDeviceLocked, this, updateItem);
    connect(DeviceManagerInstance.getDeviceInterface(), &DeviceManagerInterface::BlockDeviceUnlocked, this, updateItem);
    connect(DeviceManagerInstance.getDeviceInterface(), &DeviceManagerInterface::BlockDevicePropertyChanged, this, &ComputerItemWatcher::onDevicePropertyChanged);

    // TODO(xust): protocolDeviceAdded
    connect(DeviceManagerInstance.getDeviceInterface(), &DeviceManagerInterface::ProtocolDeviceMounted, this, &ComputerItemWatcher::onDeviceAdded);
    connect(DeviceManagerInstance.getDeviceInterface(), &DeviceManagerInterface::ProtocolDeviceUnmounted, this, [this](const QString &id) {
        auto datas = DeviceManagerInstance.invokeQueryProtocolDeviceInfo(id);
        auto &&devUrl = ComputerUtils::makeProtocolDevUrl(id);
        if (datas.value(GlobalServerDefines::DeviceProperty::kId).toString().isEmpty())   // device have been removed
            Q_EMIT this->itemRemoved(devUrl);
        else
            Q_EMIT this->itemUpdated(devUrl);
    });

    connect(DeviceManagerInstance.getDeviceInterface(), &DeviceManagerInterface::SizeUsedChanged, this, [this](const QString &id) {
        QUrl devUrl = id.startsWith(DeviceId::kBlockDeviceIdPrefix) ? ComputerUtils::makeBlockDevUrl(id) : ComputerUtils::makeProtocolDevUrl(id);
        Q_EMIT this->itemUpdated(devUrl);
    });
}

ComputerDataList ComputerItemWatcher::getUserDirItems()
{
    ComputerDataList ret;
    bool userDirAdded = false;
    ret.push_back(getGroup(kGroupDirs));

    static const QStringList udirs = { "desktop", "videos", "music", "pictures", "documents", "downloads" };
    for (auto dir : udirs) {
        QUrl url;
        url.setScheme(dfmbase::SchemeTypes::kEntry);
        url.setPath(QString("%1.%2").arg(dir).arg(SuffixInfo::kUserDir));
        //        auto info = InfoFactory::create<EntryFileInfo>(url);
        DFMEntryFileInfoPointer info(new EntryFileInfo(url));
        if (!info->exists()) continue;

        ComputerItemData data;
        data.url = url;
        data.shape = ComputerItemData::kSmallItem;
        data.info = info;
        ret.push_back(data);
        userDirAdded = true;
    }
    if (!userDirAdded)
        ret.pop_back();
    return ret;
}

ComputerDataList ComputerItemWatcher::getBlockDeviceItems(bool &hasNewItem)
{
    ComputerDataList ret;
    auto devs = DeviceManagerInstance.invokeBlockDevicesIdList({});

    for (const auto &dev : devs) {
        auto devUrl = ComputerUtils::makeBlockDevUrl(dev);
        //        auto info = InfoFactory::create<EntryFileInfo>(devUrl);
        DFMEntryFileInfoPointer info(new EntryFileInfo(devUrl));
        if (!info->exists())
            continue;

        ComputerItemData data;
        data.url = devUrl;
        data.shape = ComputerItemData::kLargeItem;
        data.info = info;
        ret.push_back(data);
        hasNewItem = true;

        addSidebarItem(devUrl, info->fileIcon().name(), info->displayName());
    }

    std::sort(ret.begin(), ret.end(), ComputerItemWatcher::typeCompare);
    return ret;
}

ComputerDataList ComputerItemWatcher::getProtocolDeviceItems(bool &hasNewItem)
{
    ComputerDataList ret;
    auto devs = DeviceManagerInstance.invokeProtolcolDevicesIdList({});

    for (const auto &dev : devs) {
        auto devUrl = ComputerUtils::makeProtocolDevUrl(dev);
        //        auto info = InfoFactory::create<EntryFileInfo>(devUrl);
        DFMEntryFileInfoPointer info(new EntryFileInfo(devUrl));
        if (!info->exists())
            continue;

        ComputerItemData data;
        data.url = devUrl;
        data.shape = ComputerItemData::kLargeItem;
        data.info = info;
        ret.push_back(data);
        hasNewItem = true;

        addSidebarItem(devUrl, info->fileIcon().name(), info->displayName());
    }

    std::sort(ret.begin(), ret.end(), ComputerItemWatcher::typeCompare);
    return ret;
}

ComputerDataList ComputerItemWatcher::getStashedProtocolItems(bool &hasNewItem)
{
    // TODO(xust)
    ComputerDataList ret;
    {
        // loop to get devices
    }

    std::sort(ret.begin(), ret.end(), ComputerItemWatcher::typeCompare);
    return ret;
}

ComputerDataList ComputerItemWatcher::getAppEntryItems(bool &hasNewItem)
{
    // TODO(xust)
    ComputerDataList ret;
    {
        // loop to get devices
    }

    std::sort(ret.begin(), ret.end(), ComputerItemWatcher::typeCompare);
    return ret;
}

/*!
 * \brief ComputerItemWatcher::getGroup, create a group item
 * \param type
 * \return
 */
ComputerItemData ComputerItemWatcher::getGroup(ComputerItemWatcher::GroupType type)
{
    ComputerItemData splitter;
    splitter.shape = ComputerItemData::kSplitterItem;
    switch (type) {
    case kGroupDirs:
        splitter.groupName = tr("My Directories");
        break;
    case kGroupDisks:
        splitter.groupName = tr("Disks");
        break;
    }
    return splitter;
}

void ComputerItemWatcher::addSidebarItem(const QUrl &url, const QString &icon, const QString &name)
{
    // additem to sidebar
    DSB_FM_USE_NAMESPACE;
    SideBar::ItemInfo sbItem;
    sbItem.group = SideBar::DefaultGroup::kDevice;
    sbItem.url = url;
    sbItem.flag = Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable;
    sbItem.iconName = icon + "-symbolic";
    sbItem.text = name;

    sbIns()->addItem(sbItem, ComputerController::cdTo, ComputerController::requestMenu, ComputerController::rename);
}

void ComputerItemWatcher::removeSidebarItem(const QUrl &url)
{
    sbIns()->removeItem(url);
}

void ComputerItemWatcher::updateSidebarItem(const QUrl &url, const QString &newName)
{
    sbIns()->updateItem(url, newName);
}

void ComputerItemWatcher::addDevice(const QString &groupName, const QUrl &url)
{
    // TODO(xust)
}

void ComputerItemWatcher::removeDevice(const QUrl &url)
{
    // TODO(xust)
}

void ComputerItemWatcher::startQueryItems()
{
    qDebug() << __PRETTY_FUNCTION__;
    auto ret = items();
    Q_EMIT itemQueryFinished(ret);
}

/*!
 * \brief ComputerItemWatcher::addGroup, add and emit itemAdded signal
 * \param name
 */
void ComputerItemWatcher::addGroup(const QString &name)
{
    ComputerItemData data;
    data.shape = ComputerItemData::kSplitterItem;
    data.groupName = name;
    emit itemAdded(data);
}

void ComputerItemWatcher::onDeviceAdded(const QString &id)
{
    QUrl devUrl;
    if (id.startsWith(DeviceId::kBlockDeviceIdPrefix))
        devUrl = ComputerUtils::makeBlockDevUrl(id);
    else
        devUrl = ComputerUtils::makeProtocolDevUrl(id);

    //    auto info = InfoFactory::create<EntryFileInfo>(devUrl);
    DFMEntryFileInfoPointer info(new EntryFileInfo(devUrl));
    if (!info->exists()) return;

    ComputerItemData data;
    data.url = devUrl;
    data.shape = ComputerItemData::kLargeItem;
    data.info = info;
    Q_EMIT this->itemAdded(data);

    addSidebarItem(devUrl, info->fileIcon().name(), info->displayName());
}

void ComputerItemWatcher::onDevicePropertyChanged(const QString &id, const QString &propertyName, const QDBusVariant &var)
{
    if (id.startsWith(DeviceId::kBlockDeviceIdPrefix)) {
        auto url = ComputerUtils::makeBlockDevUrl(id);
        // if `hintIgnore` changed to TRUE, then remove the display in view, else add it.
        if (propertyName == GlobalServerDefines::DBusDeviceProperty::kHintIgnore) {
            if (var.variant().toBool())
                Q_EMIT itemRemoved(url);
            else
                onDeviceAdded(id);
        } else {
            if (propertyName == GlobalServerDefines::DBusDeviceProperty::kIdLabel)
                updateSidebarItem(url, var.variant().toString());

            if (propertyName == GlobalServerDefines::DBusDeviceProperty::kCleartextDevice) {
                DFMEntryFileInfoPointer info(new EntryFileInfo(url));
                updateSidebarItem(url, info->displayName());
            }
            auto &&devUrl = ComputerUtils::makeBlockDevUrl(id);
            Q_EMIT itemUpdated(devUrl);
        }
    }
}

DPCOMPUTER_END_NAMESPACE
