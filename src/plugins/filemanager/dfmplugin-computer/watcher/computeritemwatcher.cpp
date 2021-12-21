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
#include "dbusservice/dbus_interface/devicemanagerdbus_interface.h"
#include "dbusservice/global_server_defines.h"
#include "utils/universalutils.h"

#include <dfm-base/base/urlroute.h>
#include <dfm-base/file/entry/entryfileinfo.h>

#include <QDebug>

DFMBASE_USE_NAMESPACE
DPCOMPUTER_BEGIN_NAMESPACE
/*!
 * \class ComputerItemWatcher
 * \brief watches the change of computer item
 */
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
    connect(UniversalUtils::deviceManager(), &DeviceManagerInterface::BlockDeviceAdded, this, [this](const QString &id) {
        auto &&devUrl = makeBlockDevUrl(id);
        DFMEntryFileInfoPointer info(new EntryFileInfo(devUrl));
        if (!info->exists()) return;

        ComputerItemData data;
        data.url = devUrl;
        data.shape = ComputerItemData::kLargeItem;
        data.info = info;
        Q_EMIT this->itemAdded(data);
    });
    connect(UniversalUtils::deviceManager(), &DeviceManagerInterface::BlockDeviceRemoved, this, [this](const QString &id) {
        auto &&devUrl = makeBlockDevUrl(id);
        Q_EMIT this->itemRemoved(devUrl);
    });

    auto updateItem = [this](const QString &id) {
        auto &&devUrl = makeBlockDevUrl(id);
        Q_EMIT this->itemUpdated(devUrl);
    };
    connect(UniversalUtils::deviceManager(), &DeviceManagerInterface::BlockDeviceMounted, this, [updateItem](const QString &id) {
        auto &&reply = UniversalUtils::deviceManager()->QueryBlockDeviceInfo(id, false);
        reply.waitForFinished();
        if (reply.isValid()) {
            auto datas = reply.value();
            auto shellDevId = datas.value(GlobalServerDefines::DeviceProperty::kCryptoBackingDevice).toString();
            if (shellDevId.length() > 1) {
                updateItem(shellDevId);
                return;
            }
        }
        updateItem(id);
    });
    connect(UniversalUtils::deviceManager(), &DeviceManagerInterface::BlockDeviceUnmounted, this, updateItem);
    connect(UniversalUtils::deviceManager(), &DeviceManagerInterface::BlockDeviceLocked, this, updateItem);
    connect(UniversalUtils::deviceManager(), &DeviceManagerInterface::BlockDeviceUnlocked, this, updateItem);

    // TODO(xust): protocolDeviceAdded
    connect(UniversalUtils::deviceManager(), &DeviceManagerInterface::ProtocolDeviceMounted, this, [this](const QString &id) {
        auto &&devUrl = makeProtocolDevUrl(id);
        DFMEntryFileInfoPointer info(new EntryFileInfo(devUrl));
        if (!info->exists()) return;

        ComputerItemData data;
        data.url = devUrl;
        data.shape = ComputerItemData::kLargeItem;
        data.info = info;
        Q_EMIT this->itemAdded(data);
    });
    connect(UniversalUtils::deviceManager(), &DeviceManagerInterface::ProtocolDeviceUnmounted, this, [this](const QString &id) {
        auto &&reply = UniversalUtils::deviceManager()->QueryProtocolDeviceInfo(id, false);
        reply.waitForFinished();
        if (reply.isValid()) {
            auto datas = reply.value();
            auto &&devUrl = makeProtocolDevUrl(id);
            if (datas.value(GlobalServerDefines::DeviceProperty::kId).toString().isEmpty())   // device have been removed
                Q_EMIT this->itemRemoved(devUrl);
            else
                Q_EMIT this->itemUpdated(devUrl);
        }
    });

    connect(UniversalUtils::deviceManager(), &DeviceManagerInterface::SizeUsedChanged, this, [this](const QString &id) {
        QUrl devUrl;
        if (id.startsWith(DeviceId::kBlockDeviceIdPrefix))
            devUrl = makeBlockDevUrl(id);
        else
            devUrl = makeProtocolDevUrl(id);
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
        DFMEntryFileInfoPointer info(new EntryFileInfo(url));
        if (!info->exists())
            continue;

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
    auto &&reply = UniversalUtils::deviceManager()->GetBlockDevicesIdList({});
    reply.waitForFinished();
    auto devs = reply.value();

    for (const auto &dev : devs) {
        auto devUrl = makeBlockDevUrl(dev);
        DFMEntryFileInfoPointer info(new EntryFileInfo(devUrl));
        if (!info->exists())
            continue;

        ComputerItemData data;
        data.url = devUrl;
        data.shape = ComputerItemData::kLargeItem;
        data.info = info;
        ret.push_back(data);
        hasNewItem = true;
    }

    std::sort(ret.begin(), ret.end(), ComputerItemWatcher::typeCompare);
    return ret;
}

ComputerDataList ComputerItemWatcher::getProtocolDeviceItems(bool &hasNewItem)
{
    ComputerDataList ret;
    auto &&reply = UniversalUtils::deviceManager()->GetProtocolDevicesIdList();
    reply.waitForFinished();
    auto devs = reply.value();

    for (const auto &dev : devs) {
        auto devUrl = makeProtocolDevUrl(dev);
        DFMEntryFileInfoPointer info(new EntryFileInfo(devUrl));
        if (!info->exists())
            continue;

        ComputerItemData data;
        data.url = devUrl;
        data.shape = ComputerItemData::kLargeItem;
        data.info = info;
        ret.push_back(data);
        hasNewItem = true;
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

QUrl ComputerItemWatcher::makeBlockDevUrl(const QString &id)
{
    QUrl devUrl;
    devUrl.setScheme(SchemeTypes::kEntry);
    auto shortenBlk = id;
    shortenBlk.remove(QString(DeviceId::kBlockDeviceIdPrefix));   // /org/freedesktop/UDisks2/block_devices/sda1 -> sda1
    auto path = QString("%1.%2").arg(shortenBlk).arg(SuffixInfo::kBlock);   // sda1.blockdev
    devUrl.setPath(path);   // entry:sda1.blockdev
    return devUrl;
}

QUrl ComputerItemWatcher::makeProtocolDevUrl(const QString &id)
{
    QUrl devUrl;
    devUrl.setScheme(SchemeTypes::kEntry);
    auto path = id.toUtf8().toBase64();
    QString encodecPath = QString("%1.%2").arg(QString(path)).arg(SuffixInfo::kProtocol);
    devUrl.setPath(encodecPath);
    return devUrl;
}
DPCOMPUTER_END_NAMESPACE
