// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMPUTERMODEL_H
#define COMPUTERMODEL_H

#include "dfmplugin_computer_global.h"
#include "watcher/computeritemwatcher.h"

#include <QAbstractItemModel>

namespace dfmplugin_computer {

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
        kDeviceIsEncryptedRole,
        kDeviceIsUnlockedRole,
        kDeviceClearDevIdRole,
        kSuffixRole,   // string: blockdev/protocoldev/userdir/...
        kProgressVisiableRole,   // bool
        kTotalSizeVisiableRole,   // bool
        kUsedSizeVisiableRole,   // bool
        kDeviceNameMaxLengthRole,   // int: the max length of name when rename a disk
        kItemShapeTypeRole,   // int: indicates the type of item, small/large/splitter/widgets
        kActionListRole,   // return the action list that item should have
        kItemIsEditingRole,   // bool: if an item is renaming
        kDeviceDescriptionRole,
        kDisplayNameIsElidedRole,   // bool
        kEditDisplayTextRole,   // string
        kItemVisibleRole
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
    ComputerItemData findItemData(const QUrl &target);
    const QList<ComputerItemData> &itemList();

    void initConnect();

Q_SIGNALS:
    void requestClearSelection(const QUrl &url);
    void requestHandleItemVisible();
    void requestUpdateIndex(const QModelIndex &index);

protected Q_SLOTS:
    void onItemAdded(const ComputerItemData &data);
    void onItemRemoved(const QUrl &url);
    void onItemUpdated(const QUrl &url);
    void updateItemInfo(int pos);
    void onItemSizeChanged(const QUrl &url, qlonglong total, qlonglong free);
    void onItemPropertyChanged(const QUrl &url, const QString &key, const QVariant &val);

    void addGroup(const ComputerItemData &data);
    void removeOrphanGroup();

private:
    QList<ComputerItemData> items;
    //    QScopedPointer<ComputerItemWatcher> watcher { nullptr };
};

}

#endif   // COMPUTERMODEL_H
