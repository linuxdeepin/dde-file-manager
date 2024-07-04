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
#include <QJsonArray>

#include <DMenu>
#include <DSettingsOption>

DPSIDEBAR_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

#define SETTING_GROUP_TOP "01_sidebar"
#define SETTING_GROUP_LV2 "01_sidebar.00_items_in_sidebar"

QMap<quint64, SideBarWidget *> SideBarHelper::kSideBarMap {};
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

/*!
 * \brief read plugins meta data for performance
 * \return map->int : additem if < 0 , insert if >=0
 */
QMap<QUrl, QPair<int, QVariantMap>> SideBarHelper::preDefineItemProperties()
{
    DPF_USE_NAMESPACE
    QMap<QUrl, QPair<int, QVariantMap>> properties;
    const auto &ptrs = LifeCycle::pluginMetaObjs([](PluginMetaObjectPointer ptr) {
        Q_ASSERT(ptr);
        const auto &data { ptr->customData() };
        if (data.isEmpty())
            return false;
        if (ptr->customData().value("SidebarDisplay").toJsonArray().isEmpty())
            return false;
        return true;
    });

    std::for_each(ptrs.begin(), ptrs.end(), [&properties](PluginMetaObjectPointer ptr) {
        const auto &array { ptr->customData().value("SidebarDisplay").toJsonArray() };
        for (int i = 0; i != array.count(); ++i) {
            const auto &obj { array.at(i).toObject() };
            QUrl url { obj.value("Url").toString() };
            if (!url.isValid())
                continue;
            QVariantMap property;
            // update later
            Qt::ItemFlags flags { Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren };
            property.insert(PropertyKey::kUrl, url);
            property.insert(PropertyKey::kDisplayName,
                            QObject::tr(qPrintable(obj.value("Name").toString())));
            property.insert(PropertyKey::kVisiableControlKey,
                            obj.value("VisiableControl").toString());
            property.insert(PropertyKey::kVisiableDisplayName,
                            obj.value("Name").toString());
            property.insert(PropertyKey::kReportName,
                            obj.value("ReportName").toString());
            property.insert(PropertyKey::kIcon,
                            QIcon::fromTheme(obj.value("Icon").toString()));
            property.insert(PropertyKey::kGroup,
                            obj.value("Group").toString());
            property.insert(PropertyKey::kQtItemFlags,
                            QVariant::fromValue(flags));

            int index { -1 };
            if (obj.contains("Pos"))
                index = obj.value("Pos").toInt();
            properties.insert(url, { index, property });
        }
    });

    return properties;
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
    if (item->group() == DefaultGroup::kTag || item->group() == DefaultGroup::kCommon) {
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

void SideBarHelper::removebindingSetting(const QString &itemVisiableSettingKey)
{
    SettingBackend::instance()->removeSerialDataKey(itemVisiableSettingKey);
    SettingBackend::instance()->removeSettingAccessor(itemVisiableSettingKey);
}

void SideBarHelper::initDefaultSettingPanel()
{
    auto ins = SettingJsonGenerator::instance();

    ins->addGroup(SETTING_GROUP_TOP, QObject::tr("Sidebar"));
    ins->addGroup(SETTING_GROUP_LV2, "Items on sidebar pane");
}

void SideBarHelper::addItemToSettingPannel(const QString &group, const QString &key, const QString &value, QMap<QString, int> *levelMap)
{
    Q_ASSERT(levelMap);
    int groupSortPrefix { 0 };
    QString groupKey;
    QString groupLabel;

    if (group == DefaultGroup::kCommon) {
        groupSortPrefix = 100;
        groupKey = QString("%1").arg(groupSortPrefix) + "_quick_access_splitter";
        groupLabel = QObject::tr("Quick access");
    } else if (group == DefaultGroup::kDevice) {
        groupSortPrefix = 200;
        groupKey = QString("%1").arg(groupSortPrefix) + "_partitions_splitter";
        groupLabel = QObject::tr("Partitions");
    } else if (group == DefaultGroup::kNetwork) {
        groupSortPrefix = 300;
        groupKey = QString("%1").arg(groupSortPrefix) + "_network_splitters";
        groupLabel = QObject::tr("Network");
    } else if (group == DefaultGroup::kTag) {
        groupSortPrefix = 400;
        groupKey = QString("%1").arg(groupSortPrefix) + "_tag_splitter";
        groupLabel = QObject::tr("Tag");
    } else {   // TODO(zhangs): gorup_other
        fmWarning() << "Invalid group:" << groupKey;
        return;
    }
    Q_ASSERT(!groupKey.isEmpty());

    QString fullGroupKey { SETTING_GROUP_LV2 + QString(".") + groupKey };
    QVariantMap groupMap = { { "key", groupKey },
                             { "name", groupLabel },
                             { "type", "sidebar-splitter" } };
    auto ins = SettingJsonGenerator::instance();

    // add group configs
    if (!ins->hasConfig(fullGroupKey)) {
        ins->addConfig(fullGroupKey, groupMap);
        SideBarInfoCacheMananger::instance()->appendLastSettingKey(fullGroupKey);
    }

    // add checkbox configs
    Q_ASSERT((*levelMap)[group] < 100);
    (*levelMap)[group] = (*levelMap)[group] + 1;
    int itemPrefix { groupSortPrefix + (*levelMap)[group] };
    QString fullItemKey { SETTING_GROUP_LV2 + QString(".") + QString("%1_%2").arg(itemPrefix).arg(key) };
    if (!ins->hasConfig(fullItemKey)) {
        ins->addCheckBoxConfig(fullItemKey, value);
        SideBarInfoCacheMananger::instance()->appendLastSettingKey(fullItemKey);
        SideBarHelper::bindSetting(fullItemKey, key);
        SideBarInfoCacheMananger::instance()->appendLastSettingBindingKey(fullItemKey);
    }
}

void SideBarHelper::removeItemFromSetting(const QString &key)
{
    auto ins = SettingJsonGenerator::instance();
    ins->removeConfig(key);
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

void SideBarHelper::openFolderInASeparateProcess(const QUrl &url)
{
    SideBarEventCaller::sendOpenWindow(url, false);
}

QMutex &SideBarHelper::mutex()
{
    static QMutex m;
    return m;
}
