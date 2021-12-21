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

DPCOMPUTER_BEGIN_NAMESPACE
typedef QList<ComputerItemData> ComputerDataList;
class ComputerItemWatcher : public QObject
{
    Q_OBJECT
public:
    explicit ComputerItemWatcher(QObject *parent = nullptr);
    virtual ~ComputerItemWatcher() override;

    ComputerDataList items();
    static bool typeCompare(const ComputerItemData &a, const ComputerItemData &b);

    enum GroupType {
        kGroupDirs,
        kGroupDisks,
    };

Q_SIGNALS:
    void itemAdded(const ComputerItemData &item);
    void itemRemoved(const QUrl &url);
    void itemUpdated(const QUrl &url);

protected Q_SLOTS:

private:
    void initConn();

    ComputerDataList getUserDirItems();
    ComputerDataList getBlockDeviceItems(bool &hasNewItem);
    ComputerDataList getProtocolDeviceItems(bool &hasNewItem);
    ComputerDataList getStashedProtocolItems(bool &hasNewItem);
    ComputerDataList getAppEntryItems(bool &hasNewItem);

    ComputerItemData getGroup(GroupType type);

    static QUrl makeBlockDevUrl(const QString &id);
    static QUrl makeProtocolDevUrl(const QString &id);

private:
    // TODO(xust)
    // appEntryWatcher
};
DPCOMPUTER_END_NAMESPACE
#endif   // COMPUTERITEMWATCHER_H
