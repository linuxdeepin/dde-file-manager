// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
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
}   // namespace plugin_events

ProtocolDeviceDisplayManager::ProtocolDeviceDisplayManager(QObject *parent)
    : QObject { parent }, d(new ProtocolDeviceDisplayManagerPrivate(this))
{
    fmDebug() << "init";
    d->init();
    fmDebug() << "init finished";
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
    if (!d->isSupportVEntry(entryUrl))
        return false;

    fmDebug() << entryUrl << "about to be inserted";

    if (displayMode() == kAggregation) {
        fmDebug() << "add aggregation item, ignore seperated item";
        QTimer::singleShot(0, this, [=] { addAggregatedItemForSeperatedOnlineItem(entryUrl); });
        return true;
    }

    return false;
}

bool ProtocolDeviceDisplayManager::hookItemsFilter(QList<QUrl> *entryUrls)
{
    if (displayMode() == kSeperate) {
        if (isShowOfflineItem())
            QTimer::singleShot(0, this, [=] { addSeperatedOfflineItems(); });
        return false;
    }

    d->removeAllSmb(entryUrls);
    QTimer::singleShot(0, this, [=] { addAggregatedItems(); });
    return true;
}

void ProtocolDeviceDisplayManager::onDevMounted(const QString &id, const QString &)
{
    if (!ProtocolUtils::isSMBFile(QUrl(id)))
        return;

    if (!isShowOfflineItem())
        return;

    // obtain the display name of `id`
    const QString &displayName = getDisplayNameOf(id);
    const QString &stdSmbPath = getStandardSmbPath(id);
    VirtualEntryDbHandler::instance()->saveAggregatedAndSperated(stdSmbPath, displayName);

    const QUrl &vEntryUrl = makeVEntryUrl(stdSmbPath);
    callItemRemove(vEntryUrl);
}

void ProtocolDeviceDisplayManager::onDevUnmounted(const QString &id)
{
    if (!ProtocolUtils::isSMBFile(QUrl(id)))
        return;

    if (displayMode() == SmbDisplayMode::kSeperate && isShowOfflineItem()) {
        const QString &stdSmbPath = getStandardSmbPath(id);
        // persistent data will be removed if "forget password" is triggered.
        // in this case, do not show the virtual entry.
        if (!VirtualEntryDbHandler::instance()->hasOfflineEntry(stdSmbPath))
            return;
        const QUrl &vEntryUrl = makeVEntryUrl(stdSmbPath);
        callItemAdd(vEntryUrl);
    } else {
        if (isShowOfflineItem())
            return;

        // only remove aggregated entry when there has no mounted share of the host.
        const QString &stdRemovedSmb = getStandardSmbPath(id);
        QString host = QUrl(stdRemovedSmb).host();
        QString removedHost = QString("smb://") + host;

        const auto &allMountedStdSmb = getStandardSmbPaths(getMountedSmb());
        bool hasMountedOfHost = std::any_of(allMountedStdSmb.cbegin(), allMountedStdSmb.cend(),
                                            [=](const QString &smb) { return smb.startsWith(removedHost); });
        if (hasMountedOfHost)
            return;
        QUrl entryUrl = makeVEntryUrl(removedHost);
        callItemRemove(entryUrl);
    }
}

void ProtocolDeviceDisplayManager::onDConfigChanged(const QString &g, const QString &k)
{
    using namespace dfm_dconfig;
    if (g == kDefaultCfgPath && k == dfm_dconfig::kShowOffline) {
        d->showOffline = DConfigManager::instance()->value(kDefaultCfgPath, kShowOffline).toBool();
        d->onShowOfflineChanged();
        fmDebug() << "showOffline changed: " << d->showOffline;
    }
}

void ProtocolDeviceDisplayManager::onJsonConfigChanged(const QString &g, const QString &k, const QVariant &v)
{
    using namespace dfm_json_config;
    if (g == kGenericAttribute && k == kSmbAggregation) {
        d->displayMode = v.toBool() ? kAggregation : kSeperate;
        d->onDisplayModeChanged();
        fmDebug() << "displayMode changed: " << d->displayMode;
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
        std::for_each(allMounted.cbegin(), allMounted.cend(), [=](const QString &devId) {
            const QString &displayName = getDisplayNameOf(devId);
            const QString &stdSmb = getStandardSmbPath(devId);
            VirtualEntryDbHandler::instance()->saveAggregatedAndSperated(stdSmb, displayName);
        });
    } else {
        // remove all visible virtual entry
        const QStringList &allStdSmb = getStandardSmbPaths(allMounted);
        QStringList allAggregated, allSeperated;
        VirtualEntryDbHandler::instance()->allSmbIDs(&allAggregated, &allSeperated);

        if (displayMode == SmbDisplayMode::kSeperate) {
            // if in seperated mode, remove all offline item
            std::for_each(allSeperated.cbegin(), allSeperated.cend(), [=](const QString &seperated) {
                auto entryUrl = makeVEntryUrl(seperated);
                callItemRemove(entryUrl);
            });
        } else {
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
    }
}

bool ProtocolDeviceDisplayManagerPrivate::isSupportVEntry(const QUrl &entryUrl)
{
    if (!showOffline && displayMode == kSeperate)
        return false;
    if (!ProtocolUtils::isSMBFile(entryUrl.path()))
        return false;
    if (!entryUrl.path().endsWith(kComputerProtocolSuffix))
        return false;
    return true;
}

bool ProtocolDeviceDisplayManagerPrivate::isSupportVEntry(const QString &devId)
{
    if (!showOffline && displayMode == kSeperate)
        return false;
    if (!ProtocolUtils::isSMBFile(devId))
        return false;
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
