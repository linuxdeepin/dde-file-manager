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

#include "dfm-base/dbusservice/global_server_defines.h"
#include "dfm-base/dbusservice/dbus_interface/devicemanagerdbus_interface.h"
#include "dfm-base/utils/devicemanager.h"
#include "dfm-base/file/entry/entryfileinfo.h"

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
    connect(DeviceManagerInstance.getDeviceInterface(), &DeviceManagerInterface::BlockDeviceAdded, this, &ComputerItemWatcher::onDeviceAdded);
    connect(DeviceManagerInstance.getDeviceInterface(), &DeviceManagerInterface::BlockDeviceRemoved, this, [this](const QString &id) {
        auto &&devUrl = makeBlockDevUrl(id);
        Q_EMIT this->itemRemoved(devUrl);
    });

    auto updateItem = [this](const QString &id) {
        auto &&devUrl = makeBlockDevUrl(id);
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
        auto &&devUrl = makeProtocolDevUrl(id);
        if (datas.value(GlobalServerDefines::DeviceProperty::kId).toString().isEmpty())   // device have been removed
            Q_EMIT this->itemRemoved(devUrl);
        else
            Q_EMIT this->itemUpdated(devUrl);
    });

    connect(DeviceManagerInstance.getDeviceInterface(), &DeviceManagerInterface::SizeUsedChanged, this, [this](const QString &id) {
        QUrl devUrl = id.startsWith(DeviceId::kBlockDeviceIdPrefix) ? makeBlockDevUrl(id) : makeProtocolDevUrl(id);
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
    auto devs = DeviceManagerInstance.invokeBlockDevicesIdList({});

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
    auto devs = DeviceManagerInstance.invokeProtolcolDevicesIdList({});

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

QString ComputerItemWatcher::getBlockDevIdByUrl(const QUrl &url)
{
    if (url.scheme() != SchemeTypes::kEntry)
        return "";
    if (!url.path().endsWith(SuffixInfo::kBlock))
        return "";

    QString suffix = QString(".%1").arg(SuffixInfo::kBlock);
    QString id = QString("%1%2").arg(DeviceId::kBlockDeviceIdPrefix).arg(url.path().remove(suffix));
    return id;
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

QString ComputerItemWatcher::getProtocolDevIdByUrl(const QUrl &url)
{
    if (url.scheme() != SchemeTypes::kEntry)
        return "";
    if (!url.path().endsWith(SuffixInfo::kProtocol))
        return "";

    QString suffix = QString(".%1").arg(SuffixInfo::kProtocol);
    QString encodecId = url.path().remove(suffix);
    QString id = QByteArray::fromBase64(encodecId.toUtf8());
    return id;
}

void ComputerItemWatcher::onDeviceAdded(const QString &id)
{
    QUrl devUrl;
    if (id.startsWith(DeviceId::kBlockDeviceIdPrefix))
        devUrl = makeBlockDevUrl(id);
    else
        devUrl = makeProtocolDevUrl(id);

    DFMEntryFileInfoPointer info(new EntryFileInfo(devUrl));
    if (!info->exists()) return;

    ComputerItemData data;
    data.url = devUrl;
    data.shape = ComputerItemData::kLargeItem;
    data.info = info;
    Q_EMIT this->itemAdded(data);
}

void ComputerItemWatcher::onDevicePropertyChanged(const QString &id, const QString &propertyName, const QDBusVariant &var)
{
    if (id.startsWith(DeviceId::kBlockDeviceIdPrefix)) {
        // if `hintIgnore` changed to TRUE, then remove the display in view, else add it.
        if (propertyName == GlobalServerDefines::DBusDeviceProperty::kHintIgnore) {
            if (var.variant().toBool())
                Q_EMIT itemRemoved(makeBlockDevUrl(id));
            else
                onDeviceAdded(id);
        } else {
            auto &&devUrl = makeBlockDevUrl(id);
            Q_EMIT itemUpdated(devUrl);
        }
    }
}

DPCOMPUTER_END_NAMESPACE
