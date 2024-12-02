// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
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
    if (d->selectFiles.count() == 0)
        return false;

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
        if (!d->stdSmb.startsWith("smb"))
            return false;
        return true;
    }

    return false;
}

bool VirtualEntryMenuScene::create(QMenu *parent)
{
    if (!parent)
        return false;

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
    if (!parent)
        return;

    using namespace menu_action_id;
    QStringList visibleActions;
    if (d->aggregatedEntrySelected)
        visibleActions << kAggregatedUnmountAll << kAggregatedForgetUnmountAll
                       << kVirtualEntryRemove;
    else if (d->seperatedEntrySelected)
        visibleActions << kCptActMount << kVirtualEntryRemove << kCptActProperty;

    if (!visibleActions.isEmpty())
        d->setActionVisible(visibleActions, parent);
}

bool VirtualEntryMenuScene::triggered(QAction *action)
{
    if (!action)
        return false;

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
    if (action == nullptr)
        return nullptr;

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
    if (!menu)
        return;

    using namespace menu_action_id;
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
    if (!triggered)
        return;

    const QString &triggeredId = triggered->property(ActionPropertyKey::kActionID).toString();
    if (triggeredId == menu_action_id::kCptActLogoutAndForget)
        actCptForget();
    else if (triggeredId == menu_action_id::kCptActMount)
        actCptMount();
}

void VirtualEntryMenuScenePrivate::actUnmountAggregatedItem(bool removeEntry)
{
    fmInfo() << "unmount all shares of" << stdSmb;
    const QStringList &devIds = protocol_display_utilities::getMountedSmb();
    const QString &stdSmbRoot = stdSmb;

    for (const auto &devId : devIds) {
        const QString &toStdSmb = protocol_display_utilities::getStandardSmbPath(devId);
        if (!toStdSmb.startsWith(stdSmb))
            continue;

        DeviceManager::instance()->unmountProtocolDevAsync(devId, {}, [=](bool ok, const DFMMOUNT::OperationErrorInfo &err) {
            fmInfo() << "unmount device:" << devId << "which represents" << toStdSmb << "result:" << ok << err.code << err.message;
            if (!ok)
                return DialogManagerInstance->showErrorDialogWhenOperateDeviceFailed(DFMBASE_NAMESPACE::DialogManager::kUnmount, err);
            if (removeEntry)
                tryRemoveAggregatedEntry(stdSmbRoot, toStdSmb);
        });
    }

    gotoDefaultPageOnUnmount();
}

void VirtualEntryMenuScenePrivate::actForgetAggregatedItem()
{
    fmInfo() << "forget saved pasword of" << stdSmb;
    computer_sidebar_event_calls::callForgetPasswd(stdSmb);
    actUnmountAggregatedItem(true);
}

void VirtualEntryMenuScenePrivate::actMountSeperatedItem()
{
    fmInfo() << "do mount for" << stdSmb;
    QString path = stdSmb;
    while (path.endsWith("/"))
        path.chop(1);

    DevMngIns->mountNetworkDeviceAsync(path, [](bool ok, const DFMMOUNT::OperationErrorInfo &err, const QString &) {
        if (ok) return;
        DialogManagerInstance->showErrorDialogWhenOperateDeviceFailed(DFMBASE_NAMESPACE::DialogManager::kMount, err);
    });
}

void VirtualEntryMenuScenePrivate::actRemoveVirtualEntry()
{
    fmInfo() << "remove offline entry of" << stdSmb;
    Q_ASSERT(selectFiles.count() > 0);

    VirtualEntryDbHandler::instance()->removeData(stdSmb);
    computer_sidebar_event_calls::callItemRemove(selectFiles.first());

    if (aggregatedEntrySelected) {
        // remove all associated seperated entry data
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
    fmDebug() << "hook on computer mount" << stdSmb;
    actMountSeperatedItem();
}

void VirtualEntryMenuScenePrivate::actCptForget()
{
    fmDebug() << "hook on computer forget" << stdSmb;
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
    VirtualEntryDbHandler::instance()->removeData(stdSmbSharePath);

    const QStringList &devIds = protocol_display_utilities::getMountedSmb();
    bool hasMounted = std::any_of(devIds.cbegin(), devIds.cend(), [=](const QString &devId) {
        const QString &toStdSmb = protocol_display_utilities::getStandardSmbPath(devId);
        return toStdSmb.startsWith(stdSmb);
    });

    if (hasMounted)
        return;

    const QUrl &aggregatedEntry = protocol_display_utilities::makeVEntryUrl(stdSmb);
    computer_sidebar_event_calls::callItemRemove(aggregatedEntry);
}
