// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dconfigupgradeunit.h"
#include "utils/upgradeutils.h"

#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

#include <QJsonObject>
#include <QJsonArray>

Q_DECLARE_LOGGING_CATEGORY(logToolUpgrade)

DFMBASE_USE_NAMESPACE
using namespace dfm_upgrade;
using namespace GlobalDConfDefines::ConfigPath;

namespace DConfigKeys {
static constexpr char kDFMMenuHidden[] { "dfm.menu.hidden" };
static constexpr char kFileManagerActionHidden[] { "dfm.menu.action.hidden" };
static constexpr char kFileDialogActionHidden[] { "dfd.menu.action.hidden" };
static constexpr char kDesktopActionHidden[] { "dd.menu.action.hidden" };

static constexpr char kSambaPermanent[] { "dfm.samba.permanent" };
static constexpr char kDiskHidden[] { "dfm.disk.hidden" };
}   // namespace DConfigKeys

DConfigUpgradeUnit::DConfigUpgradeUnit()
    : UpgradeUnit()
{
}

QString DConfigUpgradeUnit::name()
{
    return "DConfigUpgradeUnit";
}

bool DConfigUpgradeUnit::initialize(const QMap<QString, QString> &args)
{
    Q_UNUSED(args);
    return true;
}

bool DConfigUpgradeUnit::upgrade()
{
    bool ret = true;
    ret &= upgradeMenuConfigs();
    ret &= upgradeSmbConfigs();
    ret &= upgradeRecentConfigs();
    ret &= upgradeSearchConfigs();
    clearDiskHidden();
    addOldGenericSettings();

    return ret;
}

const QMap<QString, QString> &DConfigUpgradeUnit::mappedActions()
{
    static const QMap<QString, QString> mapped {
        { "Compress", "" },   // TODO(xust): this need to be completed. // TODO(liqiang)
        { "Decompress", "" },
        { "DecompressHere", "" },
        { "BookmarkRename", "" },
        { "NewWindow", "" },
        { "ClearRecent", "" },
        { "AutoMerge", "" },

        { "OpenDisk", "computer-open" },
        { "OpenDiskInNewWindow", "computer-open-in-win" },
        { "OpenDiskInNewTab", "computer-open-in-tab" },
        { "Mount", "computer-mount" },
        { "Unmount", "computer-unmount" },
        { "Eject", "computer-eject" },
        { "SafelyRemoveDrive", "computer-safely-remove" },

        { "AutoSort", "auto-arrange" },
        { "SortBy", "sort-by" },
        { "Name", "sort-by-name" },
        { "Size", "sort-by-size" },
        { "Type", "sort-by-type" },

        { "DisplayAs", "display-as" },
        { "IconSize", "icon-size" },

        { "DisplaySettings", "display-settings" },
        { "WallpaperSettings", "wallpaper-settings" },
        { "SetAsWallpaper", "set-as-wallpaper" },

        { "Property", "property" },

        { "Open", "open" },
        { "OpenFileLocation", "open-file-location" },
        { "OpenInNewWindow", "open-in-new-window" },
        { "OpenInNewTab", "open-in-new-tab" },
        { "OpenAsAdmin", "open-as-administrator" },
        { "OpenWith", "open-with" },
        { "OpenWithCustom", "open-with-custom" },
        { "OpenInTerminal", "open-in-terminal" },

        { "Cut", "cut" },
        { "Copy", "copy" },
        { "Paste", "paste" },
        { "Rename", "rename" },
        { "Delete", "delete" },
        { "CompleteDeletion", "delete" },
        { "SelectAll", "select-all" },

        { "AddToBookMark", "add-bookmark" },
        { "BookmarkRemove", "remove-bookmark" },

        { "CreateSymlink", "create-system-link" },
        { "SendToDesktop", "send-to-desktop" },
        { "SendToRemovableDisk", "send-to" },
        { "SendToBluetooth", "share-to-bluetooth" },

        { "NewFolder", "new-folder" },
        { "NewDocument", "new-document" },
        { "NewText", "new-plain-text" },

        { "Restore", "restore" },
        { "RestoreAll", "restore-all" },
        { "ClearTrash", "empty-trash" },
    };

    return mapped;
}

bool DConfigUpgradeUnit::upgradeMenuConfigs()
{
    auto upgradeActions = [](QStringList &actions) {
        for (auto &action : actions) {
            const auto &newVal = mappedActions().value(action, action);
            action = newVal.isEmpty() ? action : newVal;   // if no mapped keys, use old version.
            if (newVal.isEmpty())
                qCInfo(logToolUpgrade) << "upgrade: no mapped key, keep old value: " << action;
        }
    };

    using namespace DConfigKeys;
    auto fileManagerActionHidden = DConfigManager::instance()->value(kDefaultCfgPath, kFileManagerActionHidden).toStringList();
    auto desktopActionHidden = DConfigManager::instance()->value(kDefaultCfgPath, kDesktopActionHidden).toStringList();
    auto fileDialogActionHidden = DConfigManager::instance()->value(kDefaultCfgPath, kFileDialogActionHidden).toStringList();

    qCInfo(logToolUpgrade) << "upgrade: [old] fileManagerHiddenActions: " << fileManagerActionHidden;
    upgradeActions(fileManagerActionHidden);
    qCInfo(logToolUpgrade) << "upgrade: [new] fileManagerHiddenActions: " << fileManagerActionHidden;

    qCInfo(logToolUpgrade) << "upgrade: [old] desktopActionHidden: " << desktopActionHidden;
    upgradeActions(desktopActionHidden);
    qCInfo(logToolUpgrade) << "upgrade: [new] desktopActionHidden: " << desktopActionHidden;

    qCInfo(logToolUpgrade) << "upgrade: [old] fileDialogActionHidden: " << fileDialogActionHidden;
    upgradeActions(fileDialogActionHidden);
    qCInfo(logToolUpgrade) << "upgrade: [old] fileDialogActionHidden: " << fileDialogActionHidden;

    DConfigManager::instance()->setValue(kDefaultCfgPath, kDesktopActionHidden, desktopActionHidden);
    DConfigManager::instance()->setValue(kDefaultCfgPath, kFileManagerActionHidden, fileManagerActionHidden);
    DConfigManager::instance()->setValue(kDefaultCfgPath, kFileDialogActionHidden, fileDialogActionHidden);

    return true;
}

bool DConfigUpgradeUnit::upgradeSmbConfigs()
{
    static constexpr auto kOldKey { "AlwaysShowOfflineRemoteConnections" };
    // 0. check old
    if (checkOldGeneric(kOldKey)) {
        qCDebug(logToolUpgrade) << "SMB configuration already processed, skipping";
        return true;
    }

    // 1. read main config value
    auto oldVal = UpgradeUtils::genericAttribute(kOldKey);
    if (!oldVal.isValid()) {
        qCDebug(logToolUpgrade) << "No valid old SMB configuration found, skipping";
        return true;
    }

    auto alwaysShowSamba = oldVal.toBool();
    // 2. write to dconfig
    DConfigManager::instance()->setValue(kDefaultCfgPath, DConfigKeys::kSambaPermanent, alwaysShowSamba);
    qCInfo(logToolUpgrade) << "upgrade: set samba permanent to dconfig, value:" << alwaysShowSamba;

    // 3. remove old config
    oldGenericSettings.append(kOldKey);

    return true;
}

/*!
 * \brief DConfigUpgradeUnit::upgradeRecentConfigs
 * the Recent hidden key in dconfig in V5 is deprecated. after upgraded, the old value
 * should be synced to the new dconfig item.
 * read the old data from GenericAttribute and saved to
 * org.deepin.dde.file-manager.sidebar::itemVisiable
 * \return
 */
bool DConfigUpgradeUnit::upgradeRecentConfigs()
{
    static constexpr auto kOldKey { "ShowRecentFileEntry" };

    // 0. check old
    if (checkOldGeneric(kOldKey)) {
        qCDebug(logToolUpgrade) << "Recent file configuration already processed, skipping";
        return true;
    }

    // 1. read main config value
    auto oldValue = UpgradeUtils::genericAttribute(kOldKey);
    if (!oldValue.isValid()) {
        qCDebug(logToolUpgrade) << "No valid old recent file configuration found, skipping";
        return true;
    }

    const QString &configFile { "org.deepin.dde.file-manager.sidebar" };
    if (!DConfigManager::instance()->addConfig(configFile)) {
        qCCritical(logToolUpgrade) << "Failed to add sidebar configuration file:" << configFile;
        return false;
    }

    // 2. write to dconfig
    bool showRecent = oldValue.toBool();
    qCInfo(logToolUpgrade) << "upgrade: the old `showRecent` is" << showRecent;
    auto theSidebarVisiableList = DConfigManager::instance()->value(configFile, "itemVisiable").toMap();
    qCInfo(logToolUpgrade) << "upgrade: the new dconfig sidebar visiable list:" << theSidebarVisiableList;
    theSidebarVisiableList["recent"] = showRecent;
    DConfigManager::instance()->setValue(configFile, "itemVisiable", theSidebarVisiableList);

    // 3. remove old config
    oldGenericSettings.append(kOldKey);

    return true;
}

bool DConfigUpgradeUnit::upgradeSearchConfigs()
{
    static constexpr auto kOldKey { "IndexFullTextSearch" };

    // 0. check old
    if (checkOldGeneric(kOldKey)) {
        qCInfo(logToolUpgrade) << "Search configuration already processed, skipping";
        return true;
    }

    // 1. read main config value
    auto oldValue = UpgradeUtils::genericAttribute(kOldKey);
    if (!oldValue.isValid()) {
        qCDebug(logToolUpgrade) << "No valid old search configuration found, skipping";
        return true;
    }

    const QString &configFile { "org.deepin.dde.file-manager.search" };
    if (!DConfigManager::instance()->addConfig(configFile)) {
        qCCritical(logToolUpgrade) << "Failed to add search configuration file:" << configFile;
        return false;
    }

    bool ftsEnabled = oldValue.toBool();
    // 2. write to dconfig
    DConfigManager::instance()->setValue(configFile, "enableFullTextSearch", ftsEnabled);
    qCInfo(logToolUpgrade) << "upgrade: set search permanent to dconfig, value:" << ftsEnabled;

    // 3. remove old config
    oldGenericSettings.append(kOldKey);

    return true;
}

void DConfigUpgradeUnit::clearDiskHidden()
{
    DConfigManager::instance()->setValue(kDefaultCfgPath, DConfigKeys::kDiskHidden, QStringList());
}

void DConfigUpgradeUnit::addOldGenericSettings()
{
    if (!oldGenericSettings.isEmpty())
        UpgradeUtils::addOldGenericAttribute(QJsonArray::fromStringList(oldGenericSettings));
}

bool DConfigUpgradeUnit::checkOldGeneric(const QString &key)
{
    auto oldGeneric = UpgradeUtils::genericAttribute("OldAttributes");
    if (oldGeneric.isValid() && oldGeneric.toStringList().contains(key)) {
        qCDebug(logToolUpgrade) << "Old generic key already processed:" << key;
        return true;
    }

    return false;
}
