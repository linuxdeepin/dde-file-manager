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
        { "01_advance.04_items_in_sidebar.01_recent", "recent" },
        { "01_advance.04_items_in_sidebar.02_home", "home" },
        { "01_advance.04_items_in_sidebar.03_desktop", "desktop" },
        { "01_advance.04_items_in_sidebar.04_videos", "videos" },
        { "01_advance.04_items_in_sidebar.05_music", "music" },
        { "01_advance.04_items_in_sidebar.06_pictures", "pictures" },
        { "01_advance.04_items_in_sidebar.07_documents", "documents" },
        { "01_advance.04_items_in_sidebar.08_downloads", "downloads" },
        { "01_advance.04_items_in_sidebar.09_trash", "trash" },

        // group 10_partitions_splitter
        { "01_advance.04_items_in_sidebar.11_computer", "computer" },
        //        { "01_advance.04_items_in_sidebar.12_vault", "vault" },
        { "01_advance.04_items_in_sidebar.13_builtin", "builtin_disks" },
        { "01_advance.04_items_in_sidebar.14_loop", "loop_dev" },
        { "01_advance.04_items_in_sidebar.15_other_disks", "other_disks" },

        // group 16_network_splitters
        { "01_advance.04_items_in_sidebar.17_computers_in_lan", "computers_in_lan" },
        { "01_advance.04_items_in_sidebar.18_my_shares", "my_shares" },
        { "01_advance.04_items_in_sidebar.19_mounted_share_dirs", "mounted_share_dirs" },

        // group 20_tag_splitter
        { "01_advance.04_items_in_sidebar.21_tags", "tags" }
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
    ins->addGroup("01_advance.04_items_in_sidebar", "Items on sidebar pane");
    ins->addConfig("01_advance.04_items_in_sidebar.00_quick_access_splitter",
                   { { "key", "00_quick_access_splitter" },
                     { "name", "Quick access" },
                     { "type", "sidebar-splitter" } });
    ins->addCheckBoxConfig("01_advance.04_items_in_sidebar.01_recent",
                           "Recent");
    ins->addCheckBoxConfig("01_advance.04_items_in_sidebar.02_home",
                           "Home");
    ins->addCheckBoxConfig("01_advance.04_items_in_sidebar.03_desktop",
                           "Desktop");
    ins->addCheckBoxConfig("01_advance.04_items_in_sidebar.04_videos",
                           "Videos");
    ins->addCheckBoxConfig("01_advance.04_items_in_sidebar.05_music",
                           "Music");
    ins->addCheckBoxConfig("01_advance.04_items_in_sidebar.06_pictures",
                           "Pictures");
    ins->addCheckBoxConfig("01_advance.04_items_in_sidebar.07_documents",
                           "Documents");
    ins->addCheckBoxConfig("01_advance.04_items_in_sidebar.08_downloads",
                           "Downloads");
    ins->addCheckBoxConfig("01_advance.04_items_in_sidebar.09_trash",
                           "Trash");

    ins->addConfig("01_advance.04_items_in_sidebar.10_partitions_splitter",
                   { { "key", "10_partitions_splitter" },
                     { "name", "Partitions" },
                     { "type", "sidebar-splitter" } });
    ins->addCheckBoxConfig("01_advance.04_items_in_sidebar.11_computer",
                           "Computer");
    //    ins->addCheckBoxConfig("01_advance.04_items_in_sidebar.12_vault",
    //                           "Vault");
    ins->addCheckBoxConfig("01_advance.04_items_in_sidebar.13_builtin",
                           "Built-in disks");
    ins->addCheckBoxConfig("01_advance.04_items_in_sidebar.14_loop",
                           "Loop partitions");
    ins->addCheckBoxConfig("01_advance.04_items_in_sidebar.15_other_disks",
                           "Mounted partitions and discs");

    ins->addConfig("01_advance.04_items_in_sidebar.16_network_splitters",
                   { { "key", "16_network_splitters" },
                     { "name", "Network" },
                     { "type", "sidebar-splitter" } });
    ins->addCheckBoxConfig("01_advance.04_items_in_sidebar.17_computers_in_lan",
                           "Computers in LAN");
    ins->addCheckBoxConfig("01_advance.04_items_in_sidebar.18_my_shares",
                           "My shares");
    ins->addCheckBoxConfig("01_advance.04_items_in_sidebar.19_mounted_share_dirs",
                           "Mounted sharing folders");

    ins->addConfig("01_advance.04_items_in_sidebar.20_tag_splitter",
                   { { "key", "20_tag_splitter" },
                     { "name", "Tag" },
                     { "type", "sidebar-splitter" } });
    ins->addCheckBoxConfig("01_advance.04_items_in_sidebar.21_tags",
                           "Added tags");
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
