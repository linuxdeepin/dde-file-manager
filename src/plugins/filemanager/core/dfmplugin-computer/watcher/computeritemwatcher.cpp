// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "computeritemwatcher.h"
#include "controller/computercontroller.h"
#include "utils/computerutils.h"
#include "fileentity/appentryfileentity.h"

#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/base/configs/configsynchronizer.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/dbusservice/global_server_defines.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/utils/sysinfoutils.h>
#include <dfm-base/file/entry/entryfileinfo.h>
#include <dfm-base/file/local/localfilewatcher.h>
#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/standardpaths.h>
#include <dfm-base/dbusservice/global_server_defines.h>

#include <dfm-framework/event/event.h>

#include <QDebug>
#include <QApplication>
#include <QWindow>

using ItemClickedActionCallback = std::function<void(quint64 windowId, const QUrl &url)>;
using ContextMenuCallback = std::function<void(quint64 windowId, const QUrl &url, const QPoint &globalPos)>;
using RenameCallback = std::function<void(quint64 windowId, const QUrl &url, const QString &name)>;
using FindMeCallback = std::function<bool(const QUrl &itemUrl, const QUrl &targetUrl)>;
Q_DECLARE_METATYPE(ItemClickedActionCallback);
Q_DECLARE_METATYPE(ContextMenuCallback);
Q_DECLARE_METATYPE(RenameCallback);
Q_DECLARE_METATYPE(FindMeCallback);
Q_DECLARE_METATYPE(QList<QUrl> *);

DFMBASE_USE_NAMESPACE

namespace dfmplugin_computer {
using namespace GlobalServerDefines;

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
    initAppWatcher();
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
    int diskStartPos = ret.count();

    ret.append(getBlockDeviceItems(hasInsertNewDisk));
    ret.append(getProtocolDeviceItems(hasInsertNewDisk));
    ret.append(getAppEntryItems(hasInsertNewDisk));

    std::sort(ret.begin() + diskStartPos, ret.end(), ComputerItemWatcher::typeCompare);

    if (!hasInsertNewDisk)
        ret.pop_back();

    QList<QUrl> computerItems;
    for (const auto &item : ret)
        computerItems << item.url;

    qDebug() << "computer: [LIST] filter items BEFORE add them: " << computerItems;
    dpfHookSequence->run("dfmplugin_computer", "hook_View_ItemListFilter", &computerItems);
    qDebug() << "computer: [LIST] filter items AFTER  rmv them: " << computerItems;
    for (int i = ret.count() - 1; i >= 0; --i) {
        if (!computerItems.contains(ret[i].url)) {
            removeSidebarItem(ret[i].url);
            ret.removeAt(i);
        }
    }

    return ret;
}

ComputerDataList ComputerItemWatcher::getInitedItems()
{
    return initedDatas;
}

bool ComputerItemWatcher::typeCompare(const ComputerItemData &a, const ComputerItemData &b)
{
    return ComputerUtils::sortItem(a.info, b.info);
}

void ComputerItemWatcher::initConn()
{
    connect(appEntryWatcher.data(), &LocalFileWatcher::subfileCreated, this, [this](const QUrl &url) {
        auto appUrl = ComputerUtils::makeAppEntryUrl(url.path());
        if (!appUrl.isValid())
            return;
        this->onDeviceAdded(appUrl, getGroupId(diskGroup()), ComputerItemData::kLargeItem, false);
    });
    connect(appEntryWatcher.data(), &LocalFileWatcher::fileDeleted, this, [this](const QUrl &url) {
        auto appUrl = ComputerUtils::makeAppEntryUrl(url.path());
        if (!appUrl.isValid())
            return;
        removeDevice(appUrl);
    });

    connect(Application::instance(), &Application::genericAttributeChanged, this, &ComputerItemWatcher::onGenAttributeChanged);
    connect(DConfigManager::instance(), &DConfigManager::valueChanged, this, &ComputerItemWatcher::onDConfigChanged);

    initDeviceConn();
    connect(DevProxyMng, &DeviceProxyManager::devMngDBusRegistered, this, [this]() { startQueryItems(); });
}

void ComputerItemWatcher::initDeviceConn()
{
    connect(DevProxyMng, &DeviceProxyManager::blockDevAdded, this, &ComputerItemWatcher::onBlockDeviceAdded);
    connect(DevProxyMng, &DeviceProxyManager::blockDevRemoved, this, &ComputerItemWatcher::onBlockDeviceRemoved);
    connect(DevProxyMng, &DeviceProxyManager::blockDevMounted, this, &ComputerItemWatcher::onBlockDeviceMounted);
    connect(DevProxyMng, &DeviceProxyManager::blockDevUnmounted, this, &ComputerItemWatcher::onBlockDeviceUnmounted);
    connect(DevProxyMng, &DeviceProxyManager::blockDevLocked, this, &ComputerItemWatcher::onBlockDeviceLocked);
    connect(DevProxyMng, &DeviceProxyManager::blockDevUnlocked, this, &ComputerItemWatcher::onUpdateBlockItem);
    connect(DevProxyMng, &DeviceProxyManager::blockDevPropertyChanged, this, &ComputerItemWatcher::onDevicePropertyChangedQVar);
    connect(DevProxyMng, &DeviceProxyManager::protocolDevMounted, this, &ComputerItemWatcher::onProtocolDeviceMounted);
    connect(DevProxyMng, &DeviceProxyManager::protocolDevUnmounted, this, &ComputerItemWatcher::onProtocolDeviceUnmounted);
    connect(DevProxyMng, &DeviceProxyManager::devSizeChanged, this, &ComputerItemWatcher::onDeviceSizeChanged);
    //    connect(&DeviceManagerInstance, &DeviceManager::protocolDevAdded, this, &ComputerItemWatcher::);
    connect(DevProxyMng, &DeviceProxyManager::protocolDevRemoved, this, &ComputerItemWatcher::onProtocolDeviceRemoved);
}

void ComputerItemWatcher::initAppWatcher()
{
    QUrl extensionUrl;
    extensionUrl.setScheme(Global::Scheme::kFile);
    extensionUrl.setPath(StandardPaths::location(StandardPaths::kExtensionsAppEntryPath));
    appEntryWatcher.reset(new LocalFileWatcher(extensionUrl, this));
    appEntryWatcher->startWatcher();
}

ComputerDataList ComputerItemWatcher::getUserDirItems()
{
    ComputerDataList ret;
    bool userDirAdded = false;
    ret.push_back(getGroup(kGroupDirs));

    static const QStringList udirs = { "desktop", "videos", "music", "pictures", "documents", "downloads" };
    for (auto dir : udirs) {
        QUrl url;
        url.setScheme(DFMBASE_NAMESPACE::Global::Scheme::kEntry);
        url.setPath(QString("%1.%2").arg(dir).arg(SuffixInfo::kUserDir));
        //        auto info = InfoFactory::create<EntryFileInfo>(url);
        DFMEntryFileInfoPointer info(new EntryFileInfo(url));
        if (!info->exists()) continue;

        ComputerItemData data;
        data.url = url;
        data.shape = ComputerItemData::kSmallItem;
        data.info = info;
        data.groupId = getGroupId(userDirGroup());
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
    QStringList devs;

    qInfo() << "start obtain the blocks";
    devs = DevProxyMng->getAllBlockIds();
    qInfo() << "end obtain the blocks";

    QList<QUrl> hiddenByDConfig { disksHiddenByDConf() };
    for (const auto &dev : devs) {
        auto devUrl = ComputerUtils::makeBlockDevUrl(dev);
        DFMEntryFileInfoPointer info(new EntryFileInfo(devUrl));
        if (!info->exists())
            continue;

        ComputerItemData data;
        data.url = devUrl;
        data.shape = ComputerItemData::kLargeItem;
        data.info = info;
        data.groupId = getGroupId(diskGroup());
        ret.push_back(data);
        hasNewItem = true;

        if (info->targetUrl().isValid())
            insertUrlMapper(dev, info->targetUrl());

        if (!hiddenByDConfig.contains(devUrl))   // do not show item which hidden by dconfig
            addSidebarItem(info);
    }
    qInfo() << "end querying block info";

    return ret;
}

ComputerDataList ComputerItemWatcher::getProtocolDeviceItems(bool &hasNewItem)
{
    ComputerDataList ret;
    QStringList devs;

    qInfo() << "start obtain the protocol devices";
    devs = DevProxyMng->getAllProtocolIds();
    qInfo() << "end obtain the  protocol devices";

    for (const auto &dev : devs) {
        auto devUrl = ComputerUtils::makeProtocolDevUrl(dev);
        DFMEntryFileInfoPointer info(new EntryFileInfo(devUrl));
        if (!info->exists())
            continue;

        if (DeviceUtils::isMountPointOfDlnfs(info->targetUrl().path())) {
            qDebug() << "computer: ignore dlnfs mountpoint: " << info->targetUrl();
            continue;
        }

        ComputerItemData data;
        data.url = devUrl;
        data.shape = ComputerItemData::kLargeItem;
        data.info = info;
        data.groupId = getGroupId(diskGroup());
        ret.push_back(data);
        hasNewItem = true;

        addSidebarItem(info);
    }

    qInfo() << "end querying protocol devices info";

    return ret;
}

ComputerDataList ComputerItemWatcher::getAppEntryItems(bool &hasNewItem)
{
    static const QString appEntryPath = StandardPaths::location(StandardPaths::kExtensionsAppEntryPath);
    QDir appEntryDir(appEntryPath);
    if (!appEntryDir.exists())
        return {};

    ComputerDataList ret;

    auto entries = appEntryDir.entryList(QDir::Files);
    QStringList cmds;   // for de-duplication
    for (auto entry : entries) {
        auto entryUrl = ComputerUtils::makeAppEntryUrl(QString("%1/%2").arg(appEntryPath).arg(entry));
        if (!entryUrl.isValid())
            continue;

        DFMEntryFileInfoPointer info(new EntryFileInfo(entryUrl));
        if (!info->exists()) {
            qInfo() << "the appentry is in extension folder but not exist: " << info->urlOf(UrlInfoType::kUrl);
            continue;
        }
        QString cmd = info->extraProperty(ExtraPropertyName::kExecuteCommand).toString();
        if (cmds.contains(cmd))
            continue;
        cmds.append(cmd);

        ComputerItemData data;
        data.url = entryUrl;
        data.shape = ComputerItemData::kLargeItem;
        data.info = info;
        data.groupId = getGroupId(diskGroup());
        ret.push_back(data);
        hasNewItem = true;
    }

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
        splitter.itemName = userDirGroup();
        break;
    case kGroupDisks:
        splitter.itemName = diskGroup();
        break;
    }

    splitter.groupId = getGroupId(splitter.itemName);

    return splitter;
}

QString ComputerItemWatcher::userDirGroup()
{
    return tr("My Directories");
}

QString ComputerItemWatcher::diskGroup()
{
    return tr("Disks");
}

int ComputerItemWatcher::getGroupId(const QString &groupName)
{
    if (groupIds.contains(groupName))
        return groupIds.value(groupName);

    int id = ComputerUtils::getUniqueInteger();
    groupIds.insert(groupName, id);
    return id;
}

QList<QUrl> ComputerItemWatcher::disksHiddenByDConf()
{
    const auto &&currHiddenDisks = DConfigManager::instance()->value(kDefaultCfgPath, kKeyHideDisk).toStringList().toSet();
    const auto &&allBlockUUIDs = ComputerUtils::allValidBlockUUIDs().toSet();
    const auto &&needToBeHidden = currHiddenDisks - (currHiddenDisks - allBlockUUIDs);   // setA ∩ setB
    const auto &&devUrls = ComputerUtils::blkDevUrlByUUIDs(needToBeHidden.toList());
    return devUrls;
}

QList<QUrl> ComputerItemWatcher::disksHiddenBySettingPanel()
{
    // hidden by setting panel: no system disk
    // hidden by setting panel: no loop device
    auto systemBlocksAndLoop = DevProxyMng->getAllBlockIds(GlobalServerDefines::DeviceQueryOption::kSystem).toSet();
    auto loopOnly = DevProxyMng->getAllBlockIds(GlobalServerDefines::DeviceQueryOption::kLoop).toSet();

    bool hideSys = ComputerUtils::shouldSystemPartitionHide();
    bool hideLoop = ComputerUtils::shouldLoopPartitionsHide();

    QSet<QString> hiddenBlocks;
    if (hideSys && hideLoop)   // both hide system disks and loop devices
        hiddenBlocks = systemBlocksAndLoop;
    else if (hideSys && !hideLoop)   // hide system disks only, show loop devices
        hiddenBlocks = systemBlocksAndLoop - loopOnly;
    else if (!hideSys && hideLoop)   // show systemdisks and hide loop devices
        hiddenBlocks = loopOnly;
    else   // show nothing
        hiddenBlocks = {};

    QList<QUrl> hiddenItems;
    for (const auto &blk : hiddenBlocks)
        hiddenItems << ComputerUtils::makeBlockDevUrl(blk);
    return hiddenItems;
}

QList<QUrl> ComputerItemWatcher::hiddenPartitions()
{
    QList<QUrl> hiddenUrls;
    hiddenUrls += disksHiddenByDConf();
    hiddenUrls += disksHiddenBySettingPanel();
    hiddenUrls = QList<QUrl>::fromSet(hiddenUrls.toSet());
    return hiddenUrls;
}

void ComputerItemWatcher::cacheItem(const ComputerItemData &in)
{
    int insertAt = 0;
    bool foundGroup = false;
    auto found = std::find_if(initedDatas.cbegin(), initedDatas.cend(),
                              [in](const ComputerItemData &item) {
                                  return in.url.isValid() && item.url.isValid() && UniversalUtils::urlEquals(in.url, item.url);
                              });
    if (found != initedDatas.cend()) {
        qDebug() << "item already exists: " << in.url << in.itemName;
        return;
    }

    for (; insertAt < initedDatas.count(); insertAt++) {
        const auto &item = initedDatas.at(insertAt);
        if (item.groupId != in.groupId) {
            if (foundGroup)
                break;
            continue;
        }
        foundGroup = true;
        if (ComputerItemWatcher::typeCompare(in, item))
            break;
    }
    initedDatas.insert(insertAt, in);
}

QString ComputerItemWatcher::reportName(const QUrl &url)
{
    //    if (url.scheme() == DFMGLOBAL_NAMESPACE::Scheme::kSmb) {
    //        return "Sharing Folders";
    //    } else if (url.scheme() == DFMROOT_SCHEME) {
    //        QString strPath = url.path();
    //        if (strPath.endsWith(SUFFIX_UDISKS)) {
    //            // 截获盘符名
    //            int startIndex = strPath.indexOf("/");
    //            int endIndex = strPath.indexOf(".");
    //            int count = endIndex - startIndex - 1;
    //            QString result = strPath.mid(startIndex + 1, count);
    //            // 组装盘符绝对路径
    //            QString localPath = "/dev/" + result;
    //            // 获得块设备路径
    //            QStringList devicePaths = DDiskManager::resolveDeviceNode(localPath, {});
    //            if (!devicePaths.isEmpty()) {
    //                QString devicePath = devicePaths.first();
    //                // 获得块设备对象
    //                DBlockDevice *blDev = DDiskManager::createBlockDevice(devicePath);
    //                // 获得块设备挂载点
    //                QByteArrayList mounts = blDev->mountPoints();
    //                if (!mounts.isEmpty()) {
    //                    QString mountPath = mounts.first();
    //                    // 如果挂载点为"/"，则为系统盘
    //                    if (mountPath == "/") {
    //                        return "System Disk";
    //                    } else {   // 数据盘
    //                        return "Data Disk";
    //                    }
    //                }
    //            }
    //        } else if (strPath.endsWith(SUFFIX_GVFSMP)) {
    //            return REPORT_SHARE_DIR;
    //        }
    //    }
    return "unknow disk";
}

void ComputerItemWatcher::addSidebarItem(DFMEntryFileInfoPointer info)
{
    if (!info)
        return;

    ItemClickedActionCallback cdCb = [](quint64 winId, const QUrl &url) { ComputerControllerInstance->onOpenItem(winId, url); };
    ContextMenuCallback contextMenuCb = [](quint64 winId, const QUrl &url, const QPoint &) { ComputerControllerInstance->onMenuRequest(winId, url, true); };
    RenameCallback renameCb = [](quint64 winId, const QUrl &url, const QString &name) { ComputerControllerInstance->doRename(winId, url, name); };
    FindMeCallback findMeCb = [this](const QUrl &itemUrl, const QUrl &targetUrl) {
        if (this->routeMapper.contains(itemUrl))
            return DFMBASE_NAMESPACE::UniversalUtils::urlEquals(this->routeMapper.value(itemUrl), targetUrl);

        DFMEntryFileInfoPointer info(new EntryFileInfo(itemUrl));
        auto mntUrl = info->targetUrl();
        return dfmbase::UniversalUtils::urlEquals(mntUrl, targetUrl);
    };

    static const QStringList kItemVisiableControlKeys { "builtin_disks", "loop_dev", "other_disks", "mounted_share_dirs" };
    QString key;
    QString reportName = "Unknown Disk";
    QString subGroup = Global::Scheme::kComputer;
    if (info->extraProperty(DeviceProperty::kIsLoopDevice).toBool()) {
        key = kItemVisiableControlKeys[1];
    } else if (info->extraProperty(DeviceProperty::kHintSystem).toBool()) {
        key = kItemVisiableControlKeys[0];
        reportName = info->targetUrl().path() == "/" ? "System Disk" : "Data Disk";
    } else if (info->order() == EntryFileInfo::kOrderSmb || info->order() == EntryFileInfo::kOrderFtp) {
        key = kItemVisiableControlKeys[3];
        reportName = "Sharing Folders";
        if (info->order() == EntryFileInfo::kOrderSmb)
            subGroup = Global::Scheme::kSmb;
        else if (info->order() == EntryFileInfo::kOrderFtp)
            subGroup = Global::Scheme::kFtp;
    } else {
        key = kItemVisiableControlKeys[2];
    }

    Qt::ItemFlags flags { Qt::ItemIsEnabled | Qt::ItemIsSelectable };
    if (info->renamable())
        flags |= Qt::ItemIsEditable;
    QString iconName { info->fileIcon().name() };
    if (info->fileIcon().name().startsWith("media"))
        iconName = "media-optical-symbolic";
    else if (info->order() == EntryFileInfo::kOrderRemovableDisks)   // always display as USB icon for removable disks.
        iconName = "drive-removable-media-symbolic";
    else
        iconName += "-symbolic";

    static const QList<EntryFileInfo::EntryOrder> ejectableOrders {
        EntryFileInfo::kOrderRemovableDisks,
        EntryFileInfo::kOrderOptical,
        EntryFileInfo::kOrderSmb,
        EntryFileInfo::kOrderFtp,
        EntryFileInfo::kOrderGPhoto2,
        EntryFileInfo::kOrderMTP
    };

    QVariantMap map {
        { "Property_Key_Group", key == kItemVisiableControlKeys[3] ? "Group_Network" : "Group_Device" },
        { "Property_Key_SubGroup", subGroup },
        { "Property_Key_DisplayName", info->displayName() },
        { "Property_Key_Icon", QIcon::fromTheme(iconName) },
        { "Property_Key_FinalUrl", info->targetUrl().isValid() ? info->targetUrl() : QUrl() },
        { "Property_Key_QtItemFlags", QVariant::fromValue(flags) },
        { "Property_Key_Ejectable", ejectableOrders.contains(info->order()) },
        { "Property_Key_CallbackItemClicked", QVariant::fromValue(cdCb) },
        { "Property_Key_CallbackContextMenu", QVariant::fromValue(contextMenuCb) },
        { "Property_Key_CallbackRename", QVariant::fromValue(renameCb) },
        { "Property_Key_CallbackFindMe", QVariant::fromValue(findMeCb) },
        { "Property_Key_VisiableControl", key },
        { "Property_Key_ReportName", reportName }
    };

    dpfSlotChannel->push("dfmplugin_sidebar", "slot_Item_Add", info->urlOf(UrlInfoType::kUrl), map);
}

void ComputerItemWatcher::removeSidebarItem(const QUrl &url)
{
    dpfSlotChannel->push("dfmplugin_sidebar", "slot_Item_Remove", url);
}

void ComputerItemWatcher::handleSidebarItemsVisiable()
{
    const auto &&hiddenByDconfig = disksHiddenByDConf();

    QList<DFMEntryFileInfoPointer> visiableItems, invisiableItems;

    qInfo() << "start obtain the blocks when dconfig changed";
    auto devs = DevProxyMng->getAllBlockIds();
    qInfo() << "end obtain the blocks when dconfig changed";
    for (const auto &dev : devs) {
        auto devUrl = ComputerUtils::makeBlockDevUrl(dev);
        DFMEntryFileInfoPointer info(new EntryFileInfo(devUrl));
        if (!info->exists())
            continue;

        if (hiddenByDconfig.contains(devUrl))
            invisiableItems.append(info);
        else
            visiableItems.append(info);
    }
    qInfo() << "end querying if item should be show in sidebar";

    for (const auto &info : invisiableItems)
        removeSidebarItem(info->urlOf(UrlInfoType::kUrl));
    for (const auto &info : visiableItems)
        addSidebarItem(info);
}

void ComputerItemWatcher::insertUrlMapper(const QString &devId, const QUrl &mntUrl)
{
    QUrl devUrl;
    if (devId.startsWith(DeviceId::kBlockDeviceIdPrefix))
        devUrl = ComputerUtils::makeBlockDevUrl(devId);
    else
        devUrl = ComputerUtils::makeProtocolDevUrl(devId);
    routeMapper.insert(devUrl, mntUrl);

    if (devId.contains(QRegularExpression("sr[0-9]*$")))
        routeMapper.insert(devUrl, ComputerUtils::makeBurnUrl(devId));
}

void ComputerItemWatcher::updateSidebarItem(const QUrl &url, const QString &newName, bool editable)
{
    QVariantMap map {
        { "Property_Key_DisplayName", newName },
        { "Property_Key_Editable", editable }
    };
    dpfSlotChannel->push("dfmplugin_sidebar", "slot_Item_Update", url, map);
}

void ComputerItemWatcher::addDevice(const QString &groupName, const QUrl &url, int shape, bool addToSidebar)
{
    int groupId = addGroup(groupName);
    onDeviceAdded(url, groupId, static_cast<ComputerItemData::ShapeType>(shape), addToSidebar);
}

void ComputerItemWatcher::removeDevice(const QUrl &url)
{
    if (dpfHookSequence->run("dfmplugin_computer", "hook_View_ItemFilterOnRemove", url)) {
        qDebug() << "computer: [REMOVE] device is filtered by external plugin: " << url;
        return;
    }

    Q_EMIT itemRemoved(url);
    removeSidebarItem(url);
    auto ret = std::find_if(initedDatas.cbegin(), initedDatas.cend(), [url](const ComputerItemData &item) { return UniversalUtils::urlEquals(url, item.url); });
    if (ret != initedDatas.cend())
        initedDatas.removeAt(ret - initedDatas.cbegin());
}

void ComputerItemWatcher::startQueryItems()
{
    // if computer view is not init view, no receiver to receive the signal, cause when cd to computer view, shows empty.
    // on initialize computer view/model, get the cached items in construction.
    initedDatas = items();
    Q_EMIT itemQueryFinished(initedDatas);
    dpfSignalDispatcher->publish("dfmplugin_computer", "signal_View_Refreshed");
}

/*!
 * \brief ComputerItemWatcher::addGroup, add and emit itemAdded signal
 * \param name
 * \return a unique group id
 */
int ComputerItemWatcher::addGroup(const QString &name)
{
    auto ret = std::find_if(initedDatas.cbegin(), initedDatas.cend(), [name](const ComputerItemData &item) {
        return item.shape == ComputerItemData::kSplitterItem && item.itemName == name;
    });
    ComputerItemData data;
    if (ret != initedDatas.cend()) {
        const auto &inited = initedDatas[ret - initedDatas.cbegin()];
        data.shape = inited.shape;
        data.itemName = inited.itemName;
        data.groupId = inited.groupId;
    } else {
        data.shape = ComputerItemData::kSplitterItem;
        data.itemName = name;
        data.groupId = getGroupId(name);
        cacheItem(data);
    }

    Q_EMIT itemAdded(data);
    return data.groupId;
}

void ComputerItemWatcher::onDeviceAdded(const QUrl &devUrl, int groupId, ComputerItemData::ShapeType shape, bool needSidebarItem)
{
    DFMEntryFileInfoPointer info(new EntryFileInfo(devUrl));
    if (!info->exists()) return;

    if (dpfHookSequence->run("dfmplugin_computer", "hook_View_ItemFilterOnAdd", devUrl)) {
        qDebug() << "computer: [ADD] device is filtered by external plugin: " << devUrl;
        return;
    }

    ComputerItemData data;
    data.url = devUrl;
    data.shape = shape;
    data.info = info;
    data.groupId = groupId;
    data.itemName = info->displayName();
    Q_EMIT itemAdded(data);

    cacheItem(data);

    if (!disksHiddenByDConf().contains(devUrl) && needSidebarItem)
        addSidebarItem(info);
}

void ComputerItemWatcher::onDevicePropertyChangedQVar(const QString &id, const QString &propertyName, const QVariant &var)
{
    onDevicePropertyChangedQDBusVar(id, propertyName, QDBusVariant(var));
}

void ComputerItemWatcher::onDevicePropertyChangedQDBusVar(const QString &id, const QString &propertyName, const QDBusVariant &var)
{
    if (id.startsWith(DeviceId::kBlockDeviceIdPrefix)) {
        auto url = ComputerUtils::makeBlockDevUrl(id);
        // if `hintIgnore` changed to TRUE, then remove the display in view, else add it.
        if (propertyName == DeviceProperty::kHintIgnore) {
            if (var.variant().toBool())
                removeDevice(url);
            else
                addDevice(diskGroup(), url, ComputerItemData::kLargeItem, true);
        } else {
            auto &&devUrl = ComputerUtils::makeBlockDevUrl(id);
            if (propertyName == DeviceProperty::kOptical)
                onUpdateBlockItem(id);
            Q_EMIT itemPropertyChanged(devUrl, propertyName, var.variant());
        }

        // by default if loop device do not have filesystem interface in udisks, it will not be shown in computer,
        // and for loop devices, no blockAdded signal will be emited cause it's already existed there, so
        // watch the filesystemAdded/Removed signal to decide whether to show or hide it.
        if (propertyName == DeviceProperty::kHasFileSystem) {
            auto blkInfo = DevProxyMng->queryBlockInfo(id);
            if (blkInfo.value(DeviceProperty::kIsLoopDevice).toBool()) {
                if (var.variant().toBool())
                    onDeviceAdded(url, getGroupId(diskGroup()));
                else
                    removeDevice(url);
            }
        }
    }
}

void ComputerItemWatcher::onGenAttributeChanged(Application::GenericAttribute ga, const QVariant &value)
{
    if (ga == Application::GenericAttribute::kShowFileSystemTagOnDiskIcon) {
        Q_EMIT hideFileSystemTag(!value.toBool());
    } else if (ga == Application::GenericAttribute::kHiddenSystemPartition
               || ga == Application::GenericAttribute::kHideLoopPartitions) {
        Q_EMIT updatePartitionsVisiable();
    }
}

void ComputerItemWatcher::onDConfigChanged(const QString &cfg, const QString &cfgKey)
{
    if (cfgKey == kKeyHideDisk && cfg == kDefaultCfgPath) {
        Q_EMIT updatePartitionsVisiable();
        handleSidebarItemsVisiable();
    }
}

void ComputerItemWatcher::onBlockDeviceAdded(const QString &id)
{
    QUrl url = ComputerUtils::makeBlockDevUrl(id);
    onDeviceAdded(url, getGroupId(diskGroup()));
}

void ComputerItemWatcher::onBlockDeviceRemoved(const QString &id)
{
    auto &&devUrl = ComputerUtils::makeBlockDevUrl(id);
    removeDevice(devUrl);
    routeMapper.remove(ComputerUtils::makeBlockDevUrl(id));
}

void ComputerItemWatcher::onUpdateBlockItem(const QString &id)
{
    QUrl &&devUrl = ComputerUtils::makeBlockDevUrl(id);
    Q_EMIT this->itemUpdated(devUrl);
    auto ret = std::find_if(initedDatas.cbegin(), initedDatas.cend(), [devUrl](const ComputerItemData &data) { return data.url == devUrl; });
    if (ret != initedDatas.cend()) {
        auto item = initedDatas.at(ret - initedDatas.cbegin());
        if (item.info) {
            item.info->refresh();
            updateSidebarItem(devUrl, item.info->displayName(), item.info->renamable());
        }
    }
}

void ComputerItemWatcher::onProtocolDeviceMounted(const QString &id, const QString &mntPath)
{
    if (DeviceUtils::isMountPointOfDlnfs(mntPath)) {
        qDebug() << "computer: ignore dlnfs mountpoint: " << mntPath;
        return;
    }

    auto url = ComputerUtils::makeProtocolDevUrl(id);

    this->onDeviceAdded(url, getGroupId(diskGroup()));
}

void ComputerItemWatcher::onProtocolDeviceUnmounted(const QString &id)
{
    auto &&devUrl = ComputerUtils::makeProtocolDevUrl(id);
    removeDevice(devUrl);
    routeMapper.remove(ComputerUtils::makeProtocolDevUrl(id));
}

void ComputerItemWatcher::onDeviceSizeChanged(const QString &id, qlonglong total, qlonglong free)
{
    QUrl devUrl = id.startsWith(DeviceId::kBlockDeviceIdPrefix) ? ComputerUtils::makeBlockDevUrl(id) : ComputerUtils::makeProtocolDevUrl(id);
    Q_EMIT this->itemSizeChanged(devUrl, total, free);
}

void ComputerItemWatcher::onProtocolDeviceRemoved(const QString &id)
{
    auto &&devUrl = ComputerUtils::makeProtocolDevUrl(id);
    removeDevice(devUrl);
}

void ComputerItemWatcher::onBlockDeviceMounted(const QString &id, const QString &mntPath)
{
    Q_UNUSED(mntPath);
    auto &&datas = DevProxyMng->queryBlockInfo(id);
    auto shellDevId = datas.value(GlobalServerDefines::DeviceProperty::kCryptoBackingDevice).toString();
    onUpdateBlockItem(shellDevId.length() > 1 ? shellDevId : id);
}

void ComputerItemWatcher::onBlockDeviceUnmounted(const QString &id)
{
    routeMapper.remove(ComputerUtils::makeBlockDevUrl(id));
    onUpdateBlockItem(id);
}

void ComputerItemWatcher::onBlockDeviceLocked(const QString &id)
{
    routeMapper.remove(ComputerUtils::makeBlockDevUrl(id));
    onUpdateBlockItem(id);
}

}
