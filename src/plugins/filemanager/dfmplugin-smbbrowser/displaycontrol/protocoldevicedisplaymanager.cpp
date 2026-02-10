// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "protocoldevicedisplaymanager.h"
#include "protocoldevicedisplaymanager_p.h"
#include "displaycontrol/datahelper/virtualentrydbhandler.h"
#include "displaycontrol/info/protocolvirtualentryentity.h"
#include "displaycontrol/menu/virtualentrymenuscene.h"
#include "displaycontrol/utilities/protocoldisplayutilities.h"

#include "plugins/common/dfmplugin-menu/menu_eventinterface_helper.h"

#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/settings.h>
#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/utils/protocolutils.h>
#include <dfm-base/file/entry/entryfileinfo.h>
#include <dfm-base/base/device/devicealiasmanager.h>

#include <dfm-framework/event/event.h>

#include <QMenu>

DPSMBBROWSER_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
using namespace computer_sidebar_event_calls;
using namespace protocol_display_utilities;
using namespace ui_ventry_calls;
using namespace GlobalDConfDefines::ConfigPath;

Q_DECLARE_METATYPE(QList<QUrl> *)

namespace dfm_dconfig {
static constexpr char kShowOffline[] { "dfm.samba.permanent" };
}   // namespace dfm_dconfig

namespace dfm_json_config {
static constexpr char kSmbAggregation[] { "MergeTheEntriesOfSambaSharedFolders" };
static constexpr char kGenericAttribute[] { "GenericAttribute" };
}   // namespace dfm_json_config

namespace plugin_events {
static constexpr char kComputerMenu[] { "ComputerMenu" };

static constexpr char kComputerEventNS[] { "dfmplugin_computer" };
static constexpr char kCptHookAdd[] { "hook_View_ItemFilterOnAdd" };
static constexpr char kCptHookRemove[] { "hook_View_ItemFilterOnRemove" };
static constexpr char kCptHookListFilter[] { "hook_View_ItemListFilter" };
static constexpr char kCptSignalRenamed[] { "signal_Item_Renamed" };
}   // namespace plugin_events

ProtocolDeviceDisplayManager::ProtocolDeviceDisplayManager(QObject *parent)
    : QObject { parent }, d(new ProtocolDeviceDisplayManagerPrivate(this))
{
    fmInfo() << "Initializing ProtocolDeviceDisplayManager";
    d->init();
    fmInfo() << "ProtocolDeviceDisplayManager initialization completed";
}

ProtocolDeviceDisplayManager::~ProtocolDeviceDisplayManager()
{
}

ProtocolDeviceDisplayManager *ProtocolDeviceDisplayManager::instance()
{
    static ProtocolDeviceDisplayManager ins;
    return &ins;
}

SmbDisplayMode ProtocolDeviceDisplayManager::displayMode() const
{
    return d->displayMode;
}

bool ProtocolDeviceDisplayManager::isShowOfflineItem() const
{
    return d->showOffline;
}

bool ProtocolDeviceDisplayManager::hookItemInsert(const QUrl &entryUrl)
{
    if (!d->isSupportVEntry(entryUrl)) {
        fmDebug() << "Entry not supported for virtual entry:" << entryUrl.toString();
        return false;
    }

    fmDebug() << "Hooking item insert for supported entry:" << entryUrl.toString();

    if (displayMode() == kAggregation) {
        fmDebug() << "Display mode is aggregation, adding aggregated item for separated online item";
        QTimer::singleShot(0, this, [=] { addAggregatedItemForSeperatedOnlineItem(entryUrl); });
        return true;
    }

    return false;
}

bool ProtocolDeviceDisplayManager::hookItemsFilter(QList<QUrl> *entryUrls)
{
    if (displayMode() == kSeperate) {
        fmDebug() << "Display mode is separated, processing separated items";
        if (isShowOfflineItem()) {
            fmInfo() << "Show offline is enabled, adding separated offline items";
            QTimer::singleShot(0, this, [=] { addSeperatedOfflineItems(); });
        }
        return false;
    }

    fmInfo() << "Display mode is aggregated, removing all SMB entries and adding aggregated items";
    d->removeAllSmb(entryUrls);
    QTimer::singleShot(0, this, [=] { addAggregatedItems(); });
    return true;
}

void ProtocolDeviceDisplayManager::handleItemRenamed(const QUrl &entryUrl, const QString &name)
{
    Q_UNUSED(name)

    if (entryUrl.scheme() != "entry" || !entryUrl.path().endsWith(kVEntrySuffix)) {
        fmDebug() << "Entry not supported for entry:" << entryUrl.toString();
        return;
    }

    DFMEntryFileInfoPointer info(new EntryFileInfo(entryUrl));
    QVariantMap map {
        { "Property_Key_DisplayName", info->displayName() },
        { "Property_Key_EditDisplayText", info->editDisplayText() },
        { "Property_Key_Editable", true }
    };

    auto stdSmb = entryUrl.path().remove("." + QString(kVEntrySuffix));
    QUrl sidebarUrl(stdSmb);
    sidebarUrl.setScheme("vsmb");
    dpfSlotChannel->push("dfmplugin_sidebar", "slot_Item_Update", sidebarUrl, map);
}

void ProtocolDeviceDisplayManager::onDevMounted(const QString &id, const QString &)
{
    if (!ProtocolUtils::isSMBFile(QUrl(id))) {
        fmDebug() << "Device is not SMB file, ignoring:" << id;
        return;
    }

    if (!isShowOfflineItem()) {
        fmDebug() << "Show offline is disabled, skipping offline entry processing";
        return;
    }

    // obtain the display name of `id`
    const QString &displayName = getDisplayNameOf(id);
    const QString &stdSmbPath = getStandardSmbPath(id);
    VirtualEntryDbHandler::instance()->saveAggregatedAndSperated(stdSmbPath, displayName);

    const QUrl &vEntryUrl = makeVEntryUrl(stdSmbPath);
    callItemRemove(vEntryUrl);
}

void ProtocolDeviceDisplayManager::onDevUnmounted(const QString &id)
{
    if (!ProtocolUtils::isSMBFile(QUrl(id))) {
        fmDebug() << "Device is not SMB file, ignoring:" << id;
        return;
    }

    if (displayMode() == SmbDisplayMode::kSeperate && isShowOfflineItem()) {
        const QString &stdSmbPath = getStandardSmbPath(id);
        // persistent data will be removed if "forget password" is triggered.
        // in this case, do not show the virtual entry.
        if (!VirtualEntryDbHandler::instance()->hasOfflineEntry(stdSmbPath)) {
            fmDebug() << "No offline entry found for unmounted device, skipping virtual entry addition:" << stdSmbPath;
            return;
        }

        const QUrl &vEntryUrl = makeVEntryUrl(stdSmbPath);
        callItemAdd(vEntryUrl);
    } else {
        // only remove aggregated entry when there has no mounted share of the host.
        const QString &stdRemovedSmb = getStandardSmbPath(id);
        QString host = QUrl(stdRemovedSmb).host();
        QString removedHost = QString("smb://") + host;

        const auto &allMountedStdSmb = getStandardSmbPaths(getMountedSmb());
        bool hasMountedOfHost = std::any_of(allMountedStdSmb.cbegin(), allMountedStdSmb.cend(),
                                            [=](const QString &smb) { return smb.startsWith(removedHost); });

        if (hasMountedOfHost) {
            fmDebug() << "Host still has mounted shares, not removing aggregated entry:" << removedHost;
            return;
        } else {
            fmDebug() << "Host has no more mounted shares, forgetting password and processing entry removal:" << removedHost;
            secret_utils::forgetPasswordInSession(host);
        }

        if (isShowOfflineItem()) {
            fmDebug() << "Show offline is enabled, keeping virtual entry:" << removedHost;
            return;
        }

        QUrl entryUrl = makeVEntryUrl(removedHost);
        callItemRemove(entryUrl);
        fmInfo() << "Removed aggregated virtual entry:" << entryUrl.toString();
    }
}

void ProtocolDeviceDisplayManager::onDConfigChanged(const QString &g, const QString &k)
{
    using namespace dfm_dconfig;
    if (g == kDefaultCfgPath && k == dfm_dconfig::kShowOffline) {
        bool oldValue = d->showOffline;
        d->showOffline = DConfigManager::instance()->value(kDefaultCfgPath, kShowOffline).toBool();
        d->onShowOfflineChanged();
        fmInfo() << "Show offline setting changed from" << oldValue << "to" << d->showOffline;
    }
}

void ProtocolDeviceDisplayManager::onJsonConfigChanged(const QString &g, const QString &k, const QVariant &v)
{
    using namespace dfm_json_config;
    if (g == kGenericAttribute && k == kSmbAggregation) {
        SmbDisplayMode oldMode = d->displayMode;
        d->displayMode = v.toBool() ? kAggregation : kSeperate;
        d->onDisplayModeChanged();
        fmInfo() << "Display mode changed from" << oldMode << "to" << d->displayMode;
    }
}

void ProtocolDeviceDisplayManager::onMenuSceneAdded(const QString &scene)
{
    if (scene != plugin_events::kComputerMenu)
        return;

    bool ok = dfmplugin_menu_util::menuSceneBind(VirtualEntryMenuCreator::name(), scene);
    fmInfo() << "bind virtual entry menu to computer: " << ok;
}

void ProtocolDeviceDisplayManagerPrivate::init()
{
    using namespace dfm_json_config;
    using namespace dfm_dconfig;
    showOffline = DConfigManager::instance()->value(kDefaultCfgPath, kShowOffline).toBool();
    displayMode = Application::genericSetting()->value(kGenericAttribute, kSmbAggregation).toBool()
            ? kAggregation
            : kSeperate;

    // watch confgi changes
    q->connect(DConfigManager::instance(), &DConfigManager::valueChanged, q, &ProtocolDeviceDisplayManager::onDConfigChanged);
    q->connect(Application::genericSetting(), &Settings::valueChanged, q, &ProtocolDeviceDisplayManager::onJsonConfigChanged);

    // watch device actions
    q->connect(DevProxyMng, &DeviceProxyManager::protocolDevMounted, q, &ProtocolDeviceDisplayManager::onDevMounted);
    q->connect(DevProxyMng, &DeviceProxyManager::protocolDevUnmounted, q, &ProtocolDeviceDisplayManager::onDevUnmounted);

    // hook computer events
    using namespace plugin_events;
    dpfHookSequence->follow(kComputerEventNS, kCptHookAdd, q, &ProtocolDeviceDisplayManager::hookItemInsert);
    dpfHookSequence->follow(kComputerEventNS, kCptHookListFilter, q, &ProtocolDeviceDisplayManager::hookItemsFilter);
    dpfSignalDispatcher->subscribe(kComputerEventNS, kCptSignalRenamed, q, &ProtocolDeviceDisplayManager::handleItemRenamed);

    // regist entity info
    EntryEntityFactor::registCreator<ProtocolVirtualEntryEntity>(kVEntrySuffix);

    // regist menu and bind to computer
    using namespace dfmplugin_menu_util;
    menuSceneRegisterScene(VirtualEntryMenuCreator::name(), new VirtualEntryMenuCreator());

    if (menuSceneContains(kComputerMenu))
        menuSceneBind(VirtualEntryMenuCreator::name(), kComputerMenu);
    else
        dpfSignalDispatcher->subscribe("dfmplugin_menu", "signal_MenuScene_SceneAdded",
                                       q, &ProtocolDeviceDisplayManager::onMenuSceneAdded);
}

void ProtocolDeviceDisplayManagerPrivate::onDisplayModeChanged()
{
    callComputerRefresh();
}

void ProtocolDeviceDisplayManagerPrivate::onShowOfflineChanged()
{
    const QStringList &allMounted = getMountedSmb();
    if (showOffline) {
        fmInfo() << "Show offline enabled, saving mounted devices as virtual entries";
        std::for_each(allMounted.cbegin(), allMounted.cend(), [=](const QString &devId) {
            const QString &displayName = getDisplayNameOf(devId);
            const QString &stdSmb = getStandardSmbPath(devId);
            VirtualEntryDbHandler::instance()->saveAggregatedAndSperated(stdSmb, displayName);
        });
    } else {
        fmInfo() << "Show offline disabled, removing virtual entries";

        // remove all visible virtual entry
        const QStringList &allStdSmb = getStandardSmbPaths(allMounted);
        QStringList allAggregated, allSeperated;
        VirtualEntryDbHandler::instance()->allSmbIDs(&allAggregated, &allSeperated);

        if (displayMode == SmbDisplayMode::kSeperate) {
            fmInfo() << "Separated mode: removing all offline separated entries";
            // if in seperated mode, remove all offline item
            std::for_each(allSeperated.cbegin(), allSeperated.cend(), [=](const QString &seperated) {
                auto entryUrl = makeVEntryUrl(seperated);
                callItemRemove(entryUrl);
            });
        } else {
            fmInfo() << "Aggregated mode: removing orphan host entries";
            // else in aggregated mode, remove the entry which is just a virtual entry, no mounted share of the host
            QStringList pureVirtualEntry;
            auto hasMountedShareOf = [&allStdSmb](const QString &host) {
                return std::any_of(allStdSmb.cbegin(), allStdSmb.end(), [&](const QString &mounted) {
                    return mounted.startsWith(host);
                });
            };
            // find orphan host
            std::for_each(allAggregated.cbegin(), allAggregated.cend(), [&](const QString &host) {
                if (!hasMountedShareOf(host))
                    pureVirtualEntry << host;
            });
            std::for_each(pureVirtualEntry.cbegin(), pureVirtualEntry.cend(), [=](const QString &host) {
                auto entryUrl = makeVEntryUrl(host);
                callItemRemove(entryUrl);
            });
        }

        VirtualEntryDbHandler::instance()->clearData();
        fmInfo() << "Cleared all virtual entry data from database";
    }
}

bool ProtocolDeviceDisplayManagerPrivate::isSupportVEntry(const QUrl &entryUrl)
{
    if (!showOffline && displayMode == kSeperate) {
        fmDebug() << "Not supported: show offline disabled and display mode is separated";
        return false;
    }
    if (!ProtocolUtils::isSMBFile(entryUrl.path())) {
        fmDebug() << "Not supported: not an SMB file:" << entryUrl.path();
        return false;
    }
    if (!entryUrl.path().endsWith(kComputerProtocolSuffix)) {
        fmDebug() << "Not supported: does not end with computer protocol suffix:" << entryUrl.path();
        return false;
    }

    return true;
}

bool ProtocolDeviceDisplayManagerPrivate::isSupportVEntry(const QString &devId)
{
    if (!showOffline && displayMode == kSeperate) {
        fmDebug() << "Not supported: show offline disabled and display mode is separated";
        return false;
    }
    if (!ProtocolUtils::isSMBFile(devId)) {
        fmDebug() << "Not supported: not an SMB file:" << devId;
        return false;
    }

    return true;
}

void ProtocolDeviceDisplayManagerPrivate::removeAllSmb(QList<QUrl> *entryUrls)
{
    Q_ASSERT(entryUrls);
    for (int i = entryUrls->count() - 1; i >= 0; i--) {
        const auto &entryUrl = entryUrls->at(i);
        if (!isSupportVEntry(entryUrl))
            continue;

        entryUrls->removeAt(i);
    }
}
