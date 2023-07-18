// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "protocoldisplayutilities.h"
#include "typedefines.h"
#include "displaycontrol/datahelper/virtualentrydbhandler.h"
#include "displaycontrol/info/protocolvirtualentryentity.h"

#include "plugins/common/core/dfmplugin-menu/menu_eventinterface_helper.h"

#include <dfm-base/dfm_menu_defines.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/file/entry/entryfileinfo.h>
#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-framework/event/event.h>

#include <DMenu>

#include <QApplication>
#include <QSettings>

DPSMBBROWSER_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

namespace plugin_events {
static constexpr char kComputerEventNS[] { "dfmplugin_computer" };
static constexpr char kCptSlotAdd[] { "slot_Item_Add" };
static constexpr char kCptSlotRemove[] { "slot_Item_Remove" };
static constexpr char kCptSlotRefresh[] { "slot_View_Refresh" };
static constexpr char kCptSlotForget[] { "slot_Passwd_Clear" };

static constexpr char kSidebarEventNS[] { "dfmplugin_sidebar" };
static constexpr char kSbSlotAdd[] { "slot_Item_Add" };
static constexpr char kSbSlotRemove[] { "slot_Item_Remove" };
}   // namespace plugin_events
using namespace plugin_events;

namespace plugin_menu_names {
static constexpr char kComputerMenu[] { "ComputerMenu" };
}   // namespace plugin_menu_names
using namespace plugin_menu_names;

namespace AcName {
inline constexpr char kAcSidebarDeviceMenu[] { "sidebar_deviceitem_menu" };
}

void computer_sidebar_event_calls::callItemAdd(const QUrl &vEntryUrl)
{
    const char *kTransContext = "dfmplugin_computer::ComputerItemWatcher";
    dpfSlotChannel->push(kComputerEventNS, kCptSlotAdd,
                         qApp->translate(kTransContext, "Disks"), vEntryUrl, 1, false);

    // build params
    DFMEntryFileInfoPointer info(new EntryFileInfo(vEntryUrl));
    QVariantMap opts {
        { "Property_Key_Group", "Group_Network" },
        { "Property_Key_SubGroup", "" },
        { "Property_Key_DisplayName", info->displayName() },
        { "Property_Key_Icon", QIcon::fromTheme(info->fileIcon().name() + "-symbolic") },
        { "Property_Key_FinalUrl", info->targetUrl().isValid() ? info->targetUrl() : QUrl() },
        { "Property_Key_QtItemFlags", QVariant::fromValue(Qt::ItemIsEnabled | Qt::ItemIsSelectable) },
        { "Property_Key_Ejectable", false },
        { "Property_Key_CallbackContextMenu", QVariant::fromValue(ContextMenuCallback(sidebarMenuCall)) },
        { "Property_Key_VisiableControl", "mounted_share_dirs" }
        //        { "Property_Key_ReportName", reportName }
    };
    dpfSlotChannel->push(kSidebarEventNS, kSbSlotAdd, info->targetUrl(), opts);
}

void computer_sidebar_event_calls::callItemRemove(const QUrl &vEntryUrl)
{
    dpfSlotChannel->push(kComputerEventNS, kCptSlotRemove,
                         vEntryUrl);

    // build params
    DFMEntryFileInfoPointer info(new EntryFileInfo(vEntryUrl));
    dpfSlotChannel->push(kSidebarEventNS, kSbSlotRemove, info->targetUrl());
}

void computer_sidebar_event_calls::callComputerRefresh()
{
    // remove all virtual entry from sidebar.
    QStringList allStdSmbs = VirtualEntryDbHandler::instance()->allSmbIDs();

    const QStringList &allMounted = protocol_display_utilities::getStandardSmbPaths(protocol_display_utilities::getMountedSmb());
    for (const auto &mounted : allMounted) {
        if (!allStdSmbs.contains(mounted))
            allStdSmbs.append(mounted);
        auto host = protocol_display_utilities::getSmbHostPath(mounted);
        if (!allStdSmbs.contains(host))
            allStdSmbs.append(host);
    }

    std::for_each(allStdSmbs.cbegin(), allStdSmbs.cend(), [=](const QString &smb) {
        dpfSlotChannel->push(kSidebarEventNS, kSbSlotRemove, QUrl(smb));
    });

    dpfSlotChannel->push(kComputerEventNS, kCptSlotRefresh);
}

void computer_sidebar_event_calls::sidebarMenuCall(quint64 winId, const QUrl &url, const QPoint &pos)
{
    using namespace dfmplugin_menu_util;
    using namespace protocol_display_utilities;

    QScopedPointer<AbstractMenuScene> scene(menuSceneCreateScene(kComputerMenu));
    if (!scene) {
        pddmWar << "Create scene for computer failed: " << kComputerMenu;
        return;
    }

    QVariant selectedUrls = QVariant::fromValue<QList<QUrl>>({ makeVEntryUrl(url.toString()) });
    QVariantHash params {
        { MenuParamKey::kIsEmptyArea, false },
        { MenuParamKey::kWindowId, winId },
        { MenuParamKey::kSelectFiles, selectedUrls },
    };

    if (!scene->initialize(params))
        return;

    DTK_WIDGET_NAMESPACE::DMenu m;
#ifdef ENABLE_TESTING
    dpfSlotChannel->push("dfmplugin_utils", "slot_Accessible_SetAccessibleName",
                         qobject_cast<QWidget *>(&m), AcName::kAcSidebarDeviceMenu);
#endif
    m.setProperty("trigger-from-sidebar", true);

    scene->create(&m);
    scene->updateState(&m);

    auto act = m.exec(pos);
    if (act) {
        //        QList<QUrl> urls { url };
        //        dpfSignalDispatcher->publish("dfmplugin_computer", "signal_ReportLog_MenuData", act->text(), urls);
        scene->triggered(act);
    }
}

QUrl protocol_display_utilities::makeVEntryUrl(const QString &standardSmb)
{
    QUrl u;
    u.setScheme(Global::Scheme::kEntry);
    u.setPath(standardSmb + "." + QString(kVEntrySuffix));
    return u;
}

QStringList protocol_display_utilities::getMountedSmb()
{
    auto protoDevs = DevProxyMng->getAllProtocolIds();
    for (int i = protoDevs.count() - 1; i >= 0; i--) {
        QUrl dev(protoDevs.at(i));
        if (!DeviceUtils::isSamba(dev)) {
            protoDevs.removeAt(i);
            continue;
        }
    }
    std::sort(protoDevs.begin(), protoDevs.end());
    if (protoDevs.count() > 0)
        pddmDbg << "mounted smbs: " << protoDevs;
    return protoDevs;
}

QStringList protocol_display_utilities::getStandardSmbPaths(const QStringList &devIds)
{
    QStringList stds;
    for (auto &id : devIds)
        stds << getStandardSmbPath(id);

    return stds;
}

QString protocol_display_utilities::getSmbHostPath(const QString &devId)
{
    QUrl url(getStandardSmbPath(devId));
    url.setPath("");
    return url.toString();
}

QString protocol_display_utilities::getStandardSmbPath(const QUrl &entryUrl)
{
    QString path = entryUrl.path();
    if (!path.endsWith(kComputerProtocolSuffix))
        return "";
    path.remove("." + QString(kComputerProtocolSuffix));
    return getStandardSmbPath(path);
}

QString protocol_display_utilities::getStandardSmbPath(const QString &devId)
{
    QString id = QUrl::fromPercentEncoding(devId.toLocal8Bit());
    static const QRegularExpression kCifsSmbPrefix(R"(^file:///media/.*/smbmounts/)");

    if (!id.startsWith(Global::Scheme::kFile) || !id.contains(kCifsSmbPrefix))
        return id;

    QString dirName = id;
    dirName.remove(kCifsSmbPrefix);

    QString host, share, port;
    if (!DeviceUtils::parseSmbInfo(dirName, host, share, &port))
        return id;

    QString stdSmb;
    if (port.isEmpty())
        stdSmb = QString("smb://%1/%2/").arg(host).arg(share);
    else
        stdSmb = QString("smb://%1:%2/%3/").arg(host).arg(port).arg(share);
    return stdSmb;
}

QString protocol_display_utilities::getDisplayNameOf(const QString &devId)
{
    QUrl u;
    u.setScheme(Global::Scheme::kEntry);
    u.setPath(devId + "." + QString(kComputerProtocolSuffix));
    return getDisplayNameOf(u);
}

QString protocol_display_utilities::getDisplayNameOf(const QUrl &entryUrl)
{
    DFMEntryFileInfoPointer entry { new EntryFileInfo(entryUrl) };
    return entry ? entry->displayName() : "";
}

void ui_ventry_calls::addAggregatedItemForSeperatedOnlineItem(const QUrl &entryUrl)
{
    using namespace protocol_display_utilities;
    using namespace computer_sidebar_event_calls;

    const QString &stdSmbPath = getStandardSmbPath(entryUrl);
    const QString &smbHostPath = getSmbHostPath(stdSmbPath);
    if (smbHostPath.isEmpty())
        return;

    const QUrl &vEntryUrl = makeVEntryUrl(smbHostPath);
    callItemAdd(vEntryUrl);
}

void ui_ventry_calls::addAggregatedItems()
{
    using namespace protocol_display_utilities;
    using namespace computer_sidebar_event_calls;

    // 1. get all onlined smb
    QStringList smbs = getMountedSmb();
    // 1.1 convert to std smb
    smbs = getStandardSmbPaths(smbs);
    // 2. get all offlined smb
    smbs.append(VirtualEntryDbHandler::instance()->allSmbIDs());

    // 3. deduplicated, only keep smb root.
    QSet<QString> hostPaths;
    for (const auto &id : smbs)
        hostPaths.insert(getSmbHostPath(id));

    // 3. add aggregated item
    std::for_each(hostPaths.cbegin(), hostPaths.cend(), [=](const QString &host) {
        const QUrl &vEntryUrl = makeVEntryUrl(host);
        callItemAdd(vEntryUrl);
    });
}

void ui_ventry_calls::addSeperatedOfflineItems()
{
    using namespace protocol_display_utilities;
    using namespace computer_sidebar_event_calls;

    // 1. get all onlined smb
    QStringList smbs = getMountedSmb();
    // 1.1 convert to std smb
    smbs = getStandardSmbPaths(smbs);
    // 2. get all cached smb
    QStringList cachedSmbs = VirtualEntryDbHandler::instance()->allSmbIDs();
    // 3. get offlined smb
    std::for_each(smbs.cbegin(), smbs.cend(), [&](const QString &stdSmb) {
        cachedSmbs.removeAll(stdSmb);
    });
    // 4. seperated share only (keep the host out)
    QStringList offlinedSmbs;
    std::for_each(cachedSmbs.cbegin(), cachedSmbs.cend(), [&](const QString &stdSmb) {
        if (!QUrl(stdSmb).path().isEmpty())
            offlinedSmbs.append(stdSmb);
    });

    // 4. add seperated offline item
    std::for_each(offlinedSmbs.cbegin(), offlinedSmbs.cend(), [=](const QString &stdSmb) {
        const QUrl &vEntryUrl = makeVEntryUrl(stdSmb);
        callItemAdd(vEntryUrl);
    });
}

void computer_sidebar_event_calls::callForgetPasswd(const QString &stdSmb)
{
    dpfSlotChannel->push(kComputerEventNS, kCptSlotForget, stdSmb);
}

bool protocol_display_utilities::hasMountedShareOf(const QString &stdHost)
{
    const QStringList &allMounted = getStandardSmbPaths(getMountedSmb());
    return std::any_of(allMounted.cbegin(), allMounted.cend(), [&](const QString &stdSmb) {
        return stdSmb.startsWith(stdHost);
    });
}
