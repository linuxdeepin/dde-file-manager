// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMPUTERITEMWATCHER_H
#define COMPUTERITEMWATCHER_H

#include "dfmplugin_computer_global.h"
#include "utils/computerdatastruct.h"

#include <dfm-base/base/application/application.h>

#include <QObject>
#include <QUrl>
#include <QDBusVariant>

#define ComputerItemWatcherInstance DPCOMPUTER_NAMESPACE::ComputerItemWatcher::instance()

namespace dfmbase {
class LocalFileWatcher;
}

DFMBASE_USE_NAMESPACE
namespace dfmplugin_computer {
typedef QList<ComputerItemData> ComputerDataList;
class ComputerItemWatcher : public QObject
{
    Q_OBJECT

public:
    static ComputerItemWatcher *instance();

    ComputerDataList items();
    ComputerDataList getInitedItems();
    static bool typeCompare(const ComputerItemData &a, const ComputerItemData &b);

    enum GroupType {
        kGroupDirs,
        kGroupDisks,
        kOthers
    };

    void startQueryItems(bool async = true);

    void addDevice(const QString &groupName, const QUrl &url, int shape = ComputerItemData::kLargeItem, bool addToSidebar = true);
    void removeDevice(const QUrl &url);

    QVariantMap makeSidebarItem(DFMEntryFileInfoPointer info);
    void updateSidebarItem(const QUrl &url, const QString &newName, bool editable);
    void addSidebarItem(DFMEntryFileInfoPointer info);
    void addSidebarItem(const QUrl &url, const QVariantMap &data);
    void removeSidebarItem(const QUrl &url);
    void handleSidebarItemsVisiable();

    bool removeGroup(const QString &groupName);

    void insertUrlMapper(const QString &devId, const QUrl &mntUrl);
    void clearAsyncThread();

    static QString userDirGroup();
    static QString diskGroup();
    int getGroupId(const QString &groupName);

    static bool hideUserDir();
    static bool hide3rdEntries();
    static QList<QUrl> disksHiddenByDConf();
    static QList<QUrl> disksHiddenBySettingPanel();
    static QList<QUrl> hiddenPartitions();

    QHash<QUrl, QVariantMap> getComputerInfos() const;

public Q_SLOTS:
    void onViewRefresh();

Q_SIGNALS:
    void itemQueryFinished(const ComputerDataList &results);
    void itemAdded(const ComputerItemData &item);
    void itemRemoved(const QUrl &url);
    void itemUpdated(const QUrl &url);
    void itemPropertyChanged(const QUrl &url, const QString &property, const QVariant &var);
    void itemSizeChanged(const QUrl &url, qlonglong, qlonglong);
    void hideFileSystemTag(bool hide);
    void updatePartitionsVisiable();

protected Q_SLOTS:
    void onDeviceAdded(const QUrl &devUrl, int groupId, ComputerItemData::ShapeType shape = ComputerItemData::kLargeItem, bool needSidebarItem = true);
    void onDevicePropertyChangedQVar(const QString &id, const QString &propertyName, const QVariant &var);
    void onDevicePropertyChangedQDBusVar(const QString &id, const QString &propertyName, const QDBusVariant &var);
    void onGenAttributeChanged(Application::GenericAttribute ga, const QVariant &value);
    void onDConfigChanged(const QString &cfg, const QString &cfgKey);

    void onBlockDeviceAdded(const QString &id);
    void onBlockDeviceRemoved(const QString &id);
    void onBlockDeviceMounted(const QString &id, const QString &mntPath);
    void onBlockDeviceUnmounted(const QString &id);
    void onBlockDeviceLocked(const QString &id);
    void onUpdateBlockItem(const QString &id);
    void onProtocolDeviceMounted(const QString &id, const QString &mntPath);
    void onProtocolDeviceUnmounted(const QString &id);
    void onDeviceSizeChanged(const QString &id, qlonglong total, qlonglong free);
    void onProtocolDeviceRemoved(const QString &id);

private:
    explicit ComputerItemWatcher(QObject *parent = nullptr);
    virtual ~ComputerItemWatcher() override;

    void initConn();
    void initDeviceConn();
    void initAppWatcher();

    ComputerDataList getUserDirItems();
    ComputerDataList getBlockDeviceItems(bool *hasNewItem);
    ComputerDataList getProtocolDeviceItems(bool *hasNewItem);
    ComputerDataList getAppEntryItems(bool *hasNewItem);
    ComputerDataList getPreDefineItems();

    int addGroup(const QString &name);
    ComputerItemData getGroup(GroupType type, const QString &defaultName = "");

    void cacheItem(const ComputerItemData &in);

    QString reportName(const QUrl &url);

    QUrl findFinalUrl(DFMEntryFileInfoPointer info) const;

private:
    bool isItemQueryFinished { false };
    ComputerDataList initedDatas;
    ComputerDataList thirdItemList;
    QHash<QUrl, QVariantMap> sidebarInfos;
    QHash<QUrl, QVariantMap> computerInfos;
    QSharedPointer<DFMBASE_NAMESPACE::LocalFileWatcher> appEntryWatcher { nullptr };
    QMap<QString, int> groupIds;

    QMultiMap<QUrl, QUrl> routeMapper;
    QPointer<QFutureWatcher<ComputerDataList>> fw { nullptr };
    QList<QUrl> pendingSidebarDevUrls;   // Store pending device URLs to execute makeSidebarItem in main thread
};
}
#endif   // COMPUTERITEMWATCHER_H
