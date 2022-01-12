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
#ifndef COMPUTERMODEL_H
#define COMPUTERMODEL_H

#include "dfmplugin_computer_global.h"
#include "watcher/computeritemwatcher.h"

#include <QAbstractItemModel>

DPCOMPUTER_BEGIN_NAMESPACE

class ComputerView;
class ComputerModel : public QAbstractItemModel
{
    Q_OBJECT
    friend class ComputerView;

public:
    explicit ComputerModel(QObject *parent = nullptr);
    virtual ~ComputerModel() override;

    /*!
     * \brief The DataRoles enum
     */
    enum DataRoles {
        kSizeTotalRole = Qt::UserRole + 1,   // uint64_t
        kSizeUsageRole,
        kFileSystemRole,   // string
        kRealUrlRole,   // string: real mountpoint
        kDeviceUrlRole,   // string: device path such as entry://sdb1.localdisk
        kDeviceIsEncrypted,
        kDeviceIsUnlocked,
        kDeviceClearDevId,
        kSuffixRole,   // string: blockdev/protocoldev/userdir/...
        kProgressVisiableRole,   // bool
        kTotalSizeVisiableRole,   // bool
        kUsedSizeVisiableRole,   // bool
        kDeviceNameMaxLengthRole,   // int: the max length of name when rename a disk
        kItemShapeTypeRole,   // int: indicates the type of item, small/large/splitter/widgets
        kActionListRole,   // return the action list that item should have
        kItemIsEditing,   // bool: if an item is renaming
    };
    Q_ENUM(DataRoles)

public:
    // QAbstractItemModel interface
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    virtual QModelIndex parent(const QModelIndex &child) const override;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const override;

protected:
    int findItem(const QUrl &target);
    int findItemByClearDeviceId(const QString &id);
    int findSplitter(const QString &group);

    void initConnect();

protected Q_SLOTS:
    void onItemAdded(const ComputerItemData &data);
    void onItemRemoved(const QUrl &url);
    void onItemUpdated(const QUrl &url);
    void updateItemInfo(int pos);

private:
    ComputerView *view { nullptr };
    QList<ComputerItemData> items;
    //    QScopedPointer<ComputerItemWatcher> watcher { nullptr };
};

DPCOMPUTER_END_NAMESPACE

#endif   // COMPUTERMODEL_H
