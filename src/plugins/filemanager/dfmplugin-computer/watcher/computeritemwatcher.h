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
#ifndef COMPUTERITEMWATCHER_H
#define COMPUTERITEMWATCHER_H

#include "dfmplugin_computer_global.h"
#include "utils/computerdatastruct.h"

#include <QObject>
#include <QUrl>
#include <QDBusVariant>

#define ComputerItemWatcherInstance DPCOMPUTER_NAMESPACE::ComputerItemWatcher::instance()

DPCOMPUTER_BEGIN_NAMESPACE
typedef QList<ComputerItemData> ComputerDataList;
class ComputerItemWatcher : public QObject
{
    Q_OBJECT
public:
    static ComputerItemWatcher *instance();
    virtual ~ComputerItemWatcher() override;

    ComputerDataList items();
    ComputerDataList getInitedItems();
    static bool typeCompare(const ComputerItemData &a, const ComputerItemData &b);

    enum GroupType {
        kGroupDirs,
        kGroupDisks,
    };

    void addDevice(const QString &groupName, const QUrl &url);
    void removeDevice(const QUrl &url);


    void updateSidebarItem(const QUrl &url, const QString &newName, bool editable);
    void addSidebarItem(DFMEntryFileInfoPointer info);
    void removeSidebarItem(const QUrl &url);

public Q_SLOTS:
    void startQueryItems();

Q_SIGNALS:
    void itemQueryFinished(const ComputerDataList &results);
    void itemAdded(const ComputerItemData &item);
    void itemRemoved(const QUrl &url);
    void itemUpdated(const QUrl &url);

protected Q_SLOTS:
    void onDeviceAdded(const QString &id);
    void onDevicePropertyChanged(const QString &id, const QString &propertyName, const QDBusVariant &var);

private:
    explicit ComputerItemWatcher(QObject *parent = nullptr);
    void initConn();

    ComputerDataList getUserDirItems();
    ComputerDataList getBlockDeviceItems(bool &hasNewItem);
    ComputerDataList getProtocolDeviceItems(bool &hasNewItem);
    ComputerDataList getStashedProtocolItems(bool &hasNewItem);
    ComputerDataList getAppEntryItems(bool &hasNewItem);

    void addGroup(const QString &name);
    ComputerItemData getGroup(GroupType type);

private:
    ComputerDataList initedDatas;
    // TODO(xust)
    // appEntryWatcher
};
DPCOMPUTER_END_NAMESPACE
#endif   // COMPUTERITEMWATCHER_H
