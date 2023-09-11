// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfmplugin_sidebar_global.h"
#include "sidebarhelper.h"
#include "sidebarinfocachemananger.h"
#include "sidebaritem.h"
#include "sidebarwidget.h"

#include "events/sidebareventcaller.h"
#include "events/sidebareventreceiver.h"

#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-base/settingdialog/settingjsongenerator.h>
#include <dfm-base/settingdialog/customsettingitemregister.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/configs/settingbackend.h>
#include <dfm-base/base/configs/configsynchronizer.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/dialogs/settingsdialog/settingdialog.h>
#include <dfm-base/utils/systempathutil.h>
#include <dfm-base/utils/networkutils.h>
#include <dfm-base/dialogs/settingsdialog/settingdialog.h>
#include <dfm-base/utils/dialogmanager.h>

#include <dfm-framework/dpf.h>

#include <QLabel>

#include <DMenu>
#include <DSettingsOption>

DPSIDEBAR_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

#define SETTING_GROUP_TOP "01_sidebar"
#define SETTING_GROUP_LV2 "01_sidebar.00_items_in_sidebar"

QMap<quint64, SideBarWidget *> SideBarHelper::kSideBarMap {};
QMap<QString, SortFunc> SideBarHelper::kSortFuncs {};
bool SideBarHelper::contextMenuEnabled { true };

QList<SideBarWidget *> SideBarHelper::allSideBar()
{
    QMutexLocker locker(&SideBarHelper::mutex());
    QList<SideBarWidget *> list;
    auto keys = kSideBarMap.keys();
    for (auto k : keys)
        list.push_back(kSideBarMap[k]);

    return list;
}

SideBarWidget *SideBarHelper::findSideBarByWindowId(quint64 windowId)
{
    QMutexLocker locker(&SideBarHelper::mutex());
    if (!kSideBarMap.contains(windowId))
        return nullptr;

    return kSideBarMap[windowId];
}

void SideBarHelper::addSideBar(quint64 windowId, SideBarWidget *sideBar)
{
    QMutexLocker locker(&SideBarHelper::mutex());
    if (!kSideBarMap.contains(windowId))
        kSideBarMap.insert(windowId, sideBar);
}

void SideBarHelper::removeSideBar(quint64 windowId)
{
    QMutexLocker locker(&SideBarHelper::mutex());
    if (kSideBarMap.contains(windowId))
        kSideBarMap.remove(windowId);
}

quint64 SideBarHelper::windowId(QWidget *sender)
{
    return FMWindowsIns.findWindowId(sender);
}

SideBarItem *SideBarHelper::createItemByInfo(const ItemInfo &info)
{
    SideBarItem *item = new SideBarItem(info.icon,
                                        info.displayName,
                                        info.group,
                                        info.url);
    item->setFlags(info.flags);

    // create `unmount action` for removable device
    if (info.isEjectable) {
        DViewItemActionList lst;
        DViewItemAction *action = new DViewItemAction(Qt::AlignCenter, QSize(16, 16), QSize(), true);
        action->setIcon(QIcon::fromTheme("media-eject-symbolic"));
        action->setVisible(true);
        QObject::connect(action, &QAction::triggered, [info]() {
            SideBarEventCaller::sendEject(info.url);
        });
        lst.push_back(action);
        item->setActionList(Qt::RightEdge, lst);
    }

    return item;
}

SideBarItemSeparator *SideBarHelper::createSeparatorItem(const QString &group)
{
    SideBarItemSeparator *item = new SideBarItemSeparator(group);

    // Currently, only bookmark and tag groups support internal drag.
    // In the next stage, quick access would be instead of bookmark.
    if (item->group() == DefaultGroup::kBookmark || item->group() == DefaultGroup::kTag || item->group() == DefaultGroup::kCommon) {
        auto flags { Qt::ItemIsEnabled | Qt::ItemIsDropEnabled };
        item->setFlags(flags);
    } else
        item->setFlags(Qt::NoItemFlags);

    return item;
}

QString SideBarHelper::makeItemIdentifier(const QString &group, const QUrl &url)
{
    return group + url.url();
}

void SideBarHelper::defaultCdAction(quint64 windowId, const QUrl &url)
{
    if (!url.isEmpty())
        SideBarEventCaller::sendItemActived(windowId, url);
}

void SideBarHelper::defaultContextMenu(quint64 windowId, const QUrl &url, const QPoint &globalPos)
{
    // ref (DFMSideBarDefaultItemHandler::contextMenu)
    DMenu *menu = new DMenu;
#ifdef ENABLE_TESTING
    dpfSlotChannel->push("dfmplugin_utils", "slot_Accessible_SetAccessibleName",
                         qobject_cast<QWidget *>(menu), AcName::kAcSidebarMenuDefault);
#endif
    menu->addAction(QObject::tr("Open in new window"), [url]() {
        SideBarEventCaller::sendOpenWindow(url);
    });

    auto newTabAct = menu->addAction(QObject::tr("Open in new tab"), [windowId, url]() {
        if (NetworkUtils::instance()->checkFtpOrSmbBusy(url)) {
            DialogManager::instance()->showUnableToVistDir(url.path());
            return;
        }
        SideBarEventCaller::sendOpenTab(windowId, url);
    });

    newTabAct->setDisabled(!SideBarEventCaller::sendCheckTabAddable(windowId));

    menu->addSeparator();
    menu->addAction(QObject::tr("Properties"), [url]() {
        SideBarEventCaller::sendShowFilePropertyDialog(url);
    });
    QAction *act = menu->exec(globalPos);
    if (act) {
        QList<QUrl> urls { url };
        dpfSignalDispatcher->publish("dfmplugin_sidebar", "signal_ReportLog_MenuData", act->text(), urls);
    }
    delete menu;
}

bool SideBarHelper::registerSortFunc(const QString &subGroup, SortFunc func)
{
    if (kSortFuncs.contains(subGroup)) {
        qDebug() << subGroup << "has already been registered";
        return false;
    }
    kSortFuncs.insert(subGroup, func);
    return true;
}

SortFunc SideBarHelper::sortFunc(const QString &subGroup)
{
    return kSortFuncs.value(subGroup, nullptr);
}

void SideBarHelper::updateSideBarSelection(quint64 winId)
{
    auto all = SideBarHelper::allSideBar();
    for (auto sb : all) {
        if (!sb || SideBarHelper::windowId(sb) == winId)
            continue;
        sb->updateSelection();
    }
}

void SideBarHelper::bindSettings()
{
    static const std::map<QString, QString> kvs {
        // group 00_quick_access_splitter
        { SETTING_GROUP_LV2 ".01_recent", "recent" },
        { SETTING_GROUP_LV2 ".02_home", "home" },
        { SETTING_GROUP_LV2 ".03_desktop", "desktop" },
        { SETTING_GROUP_LV2 ".04_videos", "videos" },
        { SETTING_GROUP_LV2 ".05_music", "music" },
        { SETTING_GROUP_LV2 ".06_pictures", "pictures" },
        { SETTING_GROUP_LV2 ".07_documents", "documents" },
        { SETTING_GROUP_LV2 ".08_downloads", "downloads" },
        { SETTING_GROUP_LV2 ".09_trash", "trash" },

        // group 10_partitions_splitter
        { SETTING_GROUP_LV2 ".11_computer", "computer" },
        { SETTING_GROUP_LV2 ".13_builtin", "builtin_disks" },
        { SETTING_GROUP_LV2 ".14_loop", "loop_dev" },
        { SETTING_GROUP_LV2 ".15_other_disks", "other_disks" },

        // group 16_network_splitters
        { SETTING_GROUP_LV2 ".17_computers_in_lan", "computers_in_lan" },
        { SETTING_GROUP_LV2 ".18_my_shares", "my_shares" },
        { SETTING_GROUP_LV2 ".19_mounted_share_dirs", "mounted_share_dirs" },

        // group 20_tag_splitter
        { SETTING_GROUP_LV2 ".21_tags", "tags" }
    };

    std::for_each(kvs.begin(), kvs.end(), [](std::pair<QString, QString> pair) {
        bindSetting(pair.first, pair.second);
    });
}

void SideBarHelper::bindSetting(const QString &itemVisiableSettingKey, const QString &itemVisiableControlKey)
{
    auto getter = [](const QString &key) {
        return hiddenRules().value(key, true);
    };
    auto saver = [](const QString &key, const QVariant &val) {
        auto curr = hiddenRules();
        curr[key] = val;
        DConfigManager::instance()->setValue(ConfigInfos::kConfName, ConfigInfos::kVisiableKey, curr);
    };

    auto bindConf = [&](const QString &itemVisiableSettingKey, const QString &itemVisiableControlKey) {
        SettingBackend::instance()->addSettingAccessor(itemVisiableSettingKey,
                                                       std::bind(getter, itemVisiableControlKey),
                                                       std::bind(saver, itemVisiableControlKey, std::placeholders::_1));
    };

    // FIXME(xust): i don't know what this function do, but seems works to solve this issue.
    // commit: e634381afdb03f1f835e2e9f35d369ef782b0312
    // Bug: https://pms.uniontech.com/bug-view-156469.html
    // figure it out later.
    SettingBackend::instance()->addToSerialDataKey(itemVisiableSettingKey);
    bindConf(itemVisiableSettingKey, itemVisiableControlKey);
}

void SideBarHelper::initSettingPane()
{
    auto ins = SettingJsonGenerator::instance();
    ins->addGroup(SETTING_GROUP_TOP, QObject::tr("Sidebar"));
    ins->addGroup(SETTING_GROUP_LV2, "Items on sidebar pane");
    ins->addConfig(SETTING_GROUP_LV2 ".00_quick_access_splitter",
                   { { "key", "00_quick_access_splitter" },
                     { "name", QObject::tr("Quick access") },
                     { "type", "sidebar-splitter" } });
    ins->addCheckBoxConfig(SETTING_GROUP_LV2 ".01_recent",
                           QObject::tr("Recent"));
    ins->addCheckBoxConfig(SETTING_GROUP_LV2 ".02_home",
                           QObject::tr("Home"));
    ins->addCheckBoxConfig(SETTING_GROUP_LV2 ".03_desktop",
                           QObject::tr("Desktop"));
    ins->addCheckBoxConfig(SETTING_GROUP_LV2 ".04_videos",
                           QObject::tr("Videos"));
    ins->addCheckBoxConfig(SETTING_GROUP_LV2 ".05_music",
                           QObject::tr("Music"));
    ins->addCheckBoxConfig(SETTING_GROUP_LV2 ".06_pictures",
                           QObject::tr("Pictures"));
    ins->addCheckBoxConfig(SETTING_GROUP_LV2 ".07_documents",
                           QObject::tr("Documents"));
    ins->addCheckBoxConfig(SETTING_GROUP_LV2 ".08_downloads",
                           QObject::tr("Downloads"));
    ins->addCheckBoxConfig(SETTING_GROUP_LV2 ".09_trash",
                           QObject::tr("Trash"));

    ins->addConfig(SETTING_GROUP_LV2 ".10_partitions_splitter",
                   { { "key", "10_partitions_splitter" },
                     { "name", QObject::tr("Partitions") },
                     { "type", "sidebar-splitter" } });
    ins->addCheckBoxConfig(SETTING_GROUP_LV2 ".11_computer",
                           QObject::tr("Computer"));
    //    ins->addCheckBoxConfig(SETTING_GROUP_LV2".12_vault",
    //                           "Vault");
    ins->addCheckBoxConfig(SETTING_GROUP_LV2 ".13_builtin",
                           QObject::tr("Built-in disks"));
    ins->addCheckBoxConfig(SETTING_GROUP_LV2 ".14_loop",
                           QObject::tr("Loop partitions"));
    ins->addCheckBoxConfig(SETTING_GROUP_LV2 ".15_other_disks",
                           QObject::tr("Mounted partitions and discs"));

    ins->addConfig(SETTING_GROUP_LV2 ".16_network_splitters",
                   { { "key", "16_network_splitters" },
                     { "name", QObject::tr("Network") },
                     { "type", "sidebar-splitter" } });
    ins->addCheckBoxConfig(SETTING_GROUP_LV2 ".17_computers_in_lan",
                           QObject::tr("Computers in LAN"));
    ins->addCheckBoxConfig(SETTING_GROUP_LV2 ".18_my_shares",
                           QObject::tr("My shares"));
    ins->addCheckBoxConfig(SETTING_GROUP_LV2 ".19_mounted_share_dirs",
                           QObject::tr("Mounted sharing folders"));

    ins->addConfig(SETTING_GROUP_LV2 ".20_tag_splitter",
                   { { "key", "20_tag_splitter" },
                     { "name", QObject::tr("Tag") },
                     { "type", "sidebar-splitter" } });
    ins->addCheckBoxConfig(SETTING_GROUP_LV2 ".21_tags",
                           QObject::tr("Added tags"));
}

void SideBarHelper::registCustomSettingItem()
{
    CustomSettingItemRegister::instance()->registCustomSettingItemType("sidebar-splitter",
                                                                       [](QObject *opt) -> QPair<QWidget *, QWidget *> {
                                                                           auto option = qobject_cast<Dtk::Core::DSettingsOption *>(opt);
                                                                           auto lab = new QLabel(qApp->translate("QObject", option->name().toStdString().c_str()));
                                                                           return qMakePair(lab, nullptr);
                                                                       });
}

QVariantMap SideBarHelper::hiddenRules()
{
    return DConfigManager::instance()->value(ConfigInfos::kConfName, ConfigInfos::kVisiableKey).toMap();
}

QVariantMap SideBarHelper::groupExpandRules()
{
    return DConfigManager::instance()->value(ConfigInfos::kConfName, ConfigInfos::kGroupExpandedKey).toMap();
}

void SideBarHelper::saveGroupsStateToConfig(const QVariant &var)
{
    const QStringList keys = var.toMap().keys();

    auto &&rule = groupExpandRules();
    foreach (const QString &key, keys) {
        bool value = var.toMap().value(key).toBool();
        rule[key] = value;
    }
    DConfigManager::instance()->setValue(ConfigInfos::kConfName, ConfigInfos::kGroupExpandedKey, rule);
}

QMutex &SideBarHelper::mutex()
{
    static QMutex m;
    return m;
}
