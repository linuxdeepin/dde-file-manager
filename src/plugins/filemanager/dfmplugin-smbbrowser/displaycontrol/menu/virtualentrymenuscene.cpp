// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "virtualentrymenuscene.h"
#include "virtualentrymenuscene_p.h"
#include "displaycontrol/info/protocolvirtualentryentity.h"
#include "displaycontrol/datahelper/virtualentrydbhandler.h"
#include "displaycontrol/utilities/protocoldisplayutilities.h"

#include "plugins/common/dfmplugin-menu/menu_eventinterface_helper.h"

#include <dfm-base/dfm_menu_defines.h>
#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/utils/dialogmanager.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/base/device/devicemanager.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-base/widgets/filemanagerwindow.h>

#include <dfm-framework/event/event.h>

#include <QDebug>
#include <QMenu>

using namespace dfmplugin_smbbrowser;
DFMBASE_USE_NAMESPACE

namespace menu_action_id {
static constexpr char kVirtualEntryRemove[] { "virtual-entry-remove" };
static constexpr char kAggregatedUnmountAll[] { "aggregated-unmount" };
static constexpr char kAggregatedForgetUnmountAll[] { "aggregated-forget" };

// computer's actions
static constexpr char kCptActMount[] { "computer-mount" };
static constexpr char kCptActLogoutAndForget[] { "computer-logout-and-forget-passwd" };
static constexpr char kCptActRename[] { "computer-rename" };
static constexpr char kCptActProperty[] { "computer-property" };

// static constexpr char kCptActOpen[] { "computer-open" };
// static constexpr char kCptActOpenInNewWin[] { "computer-open-in-win" };
// static constexpr char kCptActOpenInNewTab[] { "computer-open-in-tab" };
// static constexpr char kCptActUnmount[] { "computer-unmount" };
// static constexpr char kCptActRename[] { "computer-rename" };
// static constexpr char kCptActFormat[] { "computer-format" };
// static constexpr char kCptActEject[] { "computer-eject" };
// static constexpr char kCptActErase[] { "computer-erase" };
// static constexpr char kCptActSafelyRemove[] { "computer-safely-remove" };
}   // namespace menu_action_id

VirtualEntryMenuScene::VirtualEntryMenuScene(QObject *parent)
    : AbstractMenuScene { parent }, d(new VirtualEntryMenuScenePrivate(this))
{
}

VirtualEntryMenuScene::~VirtualEntryMenuScene()
{
}

AbstractMenuScene *VirtualEntryMenuCreator::create()
{
    return new VirtualEntryMenuScene();
}

QString VirtualEntryMenuScene::name() const
{
    return VirtualEntryMenuCreator::name();
}

bool VirtualEntryMenuScene::initialize(const QVariantHash &params)
{
    d->windowId = params.value(MenuParamKey::kWindowId).toULongLong();
    d->selectFiles = params.value(MenuParamKey::kSelectFiles).value<QList<QUrl>>();
    if (d->selectFiles.count() == 0) {
        fmWarning() << "No files selected, initialization failed";
        return false;
    }

    auto subScenes = subscene();
    if (auto filterScene = dfmplugin_menu_util::menuSceneCreateScene("DConfigMenuFilter"))
        subScenes << filterScene;
    setSubscene(subScenes);

    const QUrl &entryUrl = d->selectFiles.first();
    QString path = entryUrl.path();

    if (path.endsWith(kVEntrySuffix)) {
        path.remove("." + QString(kVEntrySuffix));   // path: smb://1.2.3.4/share
        d->stdSmb = path;

        QUrl smbUrl(path);
        if (smbUrl.path().isEmpty())
            d->aggregatedEntrySelected = true;
        else
            d->seperatedEntrySelected = true;

        return true;
    } else if (path.endsWith(kComputerProtocolSuffix)) {
        path.remove("." + QString(kComputerProtocolSuffix));
        d->stdSmb = protocol_display_utilities::getStandardSmbPath(path);
        if (!d->stdSmb.startsWith("smb")) {
            fmWarning() << "Invalid SMB path after protocol conversion:" << d->stdSmb;
            return false;
        }
        return true;
    }

    fmWarning() << "Unsupported entry type, path does not match expected suffixes:" << path;
    return false;
}

bool VirtualEntryMenuScene::create(QMenu *parent)
{
    if (!parent) {
        fmWarning() << "Cannot create menu scene with null parent menu";
        return false;
    }

    connect(parent, &QMenu::triggered, this, [=](QAction *triggered) { d->hookCptActions(triggered); });

    using namespace menu_action_id;
    if (d->aggregatedEntrySelected) {
        if (protocol_display_utilities::hasMountedShareOf(d->stdSmb)) {
            d->insertActionBefore(kAggregatedUnmountAll, "", parent);
            d->insertActionBefore(kAggregatedForgetUnmountAll, "", parent);
        } else {
            d->insertActionBefore(kVirtualEntryRemove, kCptActLogoutAndForget, parent);
        }
    } else if (d->seperatedEntrySelected) {
        d->insertActionBefore(kVirtualEntryRemove, kCptActLogoutAndForget, parent);
    }
    return true;
}

void VirtualEntryMenuScene::updateState(QMenu *parent)
{
    if (!parent) {
        fmWarning() << "Cannot update menu state with null parent menu";
        return;
    }

    using namespace menu_action_id;
    QStringList visibleActions;
    if (d->aggregatedEntrySelected)
        visibleActions << kAggregatedUnmountAll << kAggregatedForgetUnmountAll
                       << kVirtualEntryRemove << kCptActRename;
    else if (d->seperatedEntrySelected)
        visibleActions << kCptActMount << kVirtualEntryRemove << kCptActRename << kCptActProperty;

    if (!visibleActions.isEmpty())
        d->setActionVisible(visibleActions, parent);
}

bool VirtualEntryMenuScene::triggered(QAction *action)
{
    if (!action) {
        fmWarning() << "Null action triggered";
        return false;
    }

    using namespace menu_action_id;
    auto key = action->property(ActionPropertyKey::kActionID).toString();
    bool handled = false;
    if (key == kAggregatedUnmountAll && (handled = true))
        d->actUnmountAggregatedItem();
    else if (key == kAggregatedForgetUnmountAll && (handled = true))
        d->actForgetAggregatedItem();
    else if (key == kVirtualEntryRemove && (handled = true))
        d->actRemoveVirtualEntry();

    return handled || AbstractMenuScene::triggered(action);
}

AbstractMenuScene *VirtualEntryMenuScene::scene(QAction *action) const
{
    if (action == nullptr) {
        fmDebug() << "Null action passed to scene method";
        return nullptr;
    }

    if (!d->predicateAction.key(action).isEmpty())
        return const_cast<VirtualEntryMenuScene *>(this);

    return AbstractMenuScene::scene(action);
}

VirtualEntryMenuScenePrivate::VirtualEntryMenuScenePrivate(VirtualEntryMenuScene *qq)
    : AbstractMenuScenePrivate(qq)
{
    using namespace menu_action_id;
    predicateName[kAggregatedUnmountAll] = tr("Unmount");
    predicateName[kAggregatedForgetUnmountAll] = tr("Clear saved password and unmount");
    predicateName[kVirtualEntryRemove] = tr("Remove");
}

void VirtualEntryMenuScenePrivate::setActionVisible(const QStringList &visibleActions, QMenu *menu)
{
    if (!menu) {
        fmWarning() << "Cannot set action visibility with null menu";
        return;
    }

    using namespace menu_action_id;
    int visibleCount = 0;
    for (const auto &act : menu->actions()) {
        const QString &id = act->property(ActionPropertyKey::kActionID).toString();
        act->setVisible(visibleActions.contains(id) || act->isSeparator());
    }
}

void VirtualEntryMenuScenePrivate::insertActionBefore(const QString &inserted, const QString &before, QMenu *menu)
{
    Q_ASSERT(menu);
    for (const auto &action : menu->actions()) {
        if (action->property(ActionPropertyKey::kActionID).toString() != before)
            continue;

        // found
        QAction *act = new QAction(predicateName[inserted], menu);
        act->setProperty(ActionPropertyKey::kActionID, inserted);
        menu->insertAction(action, act);
        predicateAction[inserted] = act;
        return;
    }

    auto act = menu->addAction(predicateName[inserted]);
    predicateAction[inserted] = act;
    act->setProperty(ActionPropertyKey::kActionID, inserted);
}

void VirtualEntryMenuScenePrivate::hookCptActions(QAction *triggered)
{
    if (!triggered) {
        fmWarning() << "Null action passed to hookCptActions";
        return;
    }

    const QString &triggeredId = triggered->property(ActionPropertyKey::kActionID).toString();
    if (triggeredId == menu_action_id::kCptActLogoutAndForget)
        actCptForget();
    else if (triggeredId == menu_action_id::kCptActMount)
        actCptMount();
}

void VirtualEntryMenuScenePrivate::actUnmountAggregatedItem(bool removeEntry)
{
    fmInfo() << "Unmounting all shares of" << stdSmb << "removeEntry:" << removeEntry;

    const QStringList &devIds = protocol_display_utilities::getMountedSmb();
    const QString &stdSmbRoot = stdSmb;

    for (const auto &devId : devIds) {
        const QString &toStdSmb = protocol_display_utilities::getStandardSmbPath(devId);
        if (!toStdSmb.startsWith(stdSmb))
            continue;

        DeviceManager::instance()->unmountProtocolDevAsync(devId, {}, [=](bool ok, const DFMMOUNT::OperationErrorInfo &err) {
            if (ok) {
                fmInfo() << "Successfully unmounted device:" << devId << "SMB path:" << toStdSmb;
                if (removeEntry)
                    tryRemoveAggregatedEntry(stdSmbRoot, toStdSmb);
            } else {
                fmCritical() << "Failed to unmount device:" << devId << "SMB path:" << toStdSmb
                             << "error code:" << err.code << "message:" << err.message;
                DialogManagerInstance->showErrorDialogWhenOperateDeviceFailed(DFMBASE_NAMESPACE::DialogManager::kUnmount, err);
            }
        });
    }

    gotoDefaultPageOnUnmount();
}

void VirtualEntryMenuScenePrivate::actForgetAggregatedItem()
{
    fmInfo() << "Forgetting saved password and unmounting all shares of" << stdSmb;

    computer_sidebar_event_calls::callForgetPasswd(stdSmb);
    actUnmountAggregatedItem(true);
}

void VirtualEntryMenuScenePrivate::actMountSeperatedItem()
{
    fmInfo() << "Mounting separated item:" << stdSmb;

    QString path = stdSmb;
    while (path.endsWith("/"))
        path.chop(1);

    fmDebug() << "Normalized mount path:" << path;

    DevMngIns->mountNetworkDeviceAsync(path, [=](bool ok, const DFMMOUNT::OperationErrorInfo &err, const QString &mountPoint) {
        if (ok) {
            fmInfo() << "Successfully mounted network device:" << path << "mount point:" << mountPoint;
        } else {
            fmCritical() << "Failed to mount network device:" << path
                         << "error code:" << err.code << "message:" << err.message;
            DialogManagerInstance->showErrorDialogWhenOperateDeviceFailed(DFMBASE_NAMESPACE::DialogManager::kMount, err);
        }
    });
}

void VirtualEntryMenuScenePrivate::actRemoveVirtualEntry()
{
    fmInfo() << "Removing offline entry for:" << stdSmb;
    Q_ASSERT(selectFiles.count() > 0);

    VirtualEntryDbHandler::instance()->removeData(stdSmb);
    computer_sidebar_event_calls::callItemRemove(selectFiles.first());

    if (aggregatedEntrySelected) {
        fmDebug() << "Removing associated separated entries for aggregated entry";

        // remove all associated separated entry data
        QStringList seperatedSmbs;
        VirtualEntryDbHandler::instance()->allSmbIDs(nullptr, &seperatedSmbs);
        QString host = stdSmb;
        if (!host.endsWith("/"))
            host.append("/");

        std::for_each(seperatedSmbs.cbegin(), seperatedSmbs.cend(), [&](const QString &seperated) {
            if (seperated.startsWith(host))
                VirtualEntryDbHandler::instance()->removeData(seperated);
        });

        computer_sidebar_event_calls::callForgetPasswd(stdSmb);
        gotoDefaultPageOnUnmount();
    }
}

void VirtualEntryMenuScenePrivate::actCptMount()
{
    fmDebug() << "Hooking computer mount action for:" << stdSmb;
    actMountSeperatedItem();
}

void VirtualEntryMenuScenePrivate::actCptForget()
{
    fmDebug() << "Hooking computer forget action for:" << stdSmb;

    // do remove the cached data.
    VirtualEntryDbHandler::instance()->removeData(stdSmb);
}

void VirtualEntryMenuScenePrivate::gotoDefaultPageOnUnmount()
{
    const QUrl &defaultUrl = Application::instance()->appAttribute(Application::kUrlOfNewWindow).toUrl();
    const auto &&winIds = FileManagerWindowsManager::instance().windowIdList();
    for (const auto &winId : winIds) {
        const auto &window = FileManagerWindowsManager::instance().findWindowById(winId);
        if (!window)
            continue;

        const QUrl &urlOfWin = window->currentUrl();
        if (!UniversalUtils::urlEquals(urlOfWin, QUrl(stdSmb)))
            continue;

        dpfSignalDispatcher->publish(GlobalEventType::kChangeCurrentUrl, winId, defaultUrl);
    }
}

void VirtualEntryMenuScenePrivate::tryRemoveAggregatedEntry(const QString &stdSmb, const QString &stdSmbSharePath)
{
    fmDebug() << "Attempting to remove aggregated entry:" << stdSmb << "share path:" << stdSmbSharePath;

    VirtualEntryDbHandler::instance()->removeData(stdSmbSharePath);

    const QStringList &devIds = protocol_display_utilities::getMountedSmb();
    bool hasMounted = std::any_of(devIds.cbegin(), devIds.cend(), [=](const QString &devId) {
        const QString &toStdSmb = protocol_display_utilities::getStandardSmbPath(devId);
        return toStdSmb.startsWith(stdSmb);
    });

    if (hasMounted) {
        fmDebug() << "Still has mounted shares, not removing aggregated entry:" << stdSmb;
        return;
    }

    fmInfo() << "No mounted shares remain, removing aggregated entry:" << stdSmb;
    const QUrl &aggregatedEntry = protocol_display_utilities::makeVEntryUrl(stdSmb);
    computer_sidebar_event_calls::callItemRemove(aggregatedEntry);
}
