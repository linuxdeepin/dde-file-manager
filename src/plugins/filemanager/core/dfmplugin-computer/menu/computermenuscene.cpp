// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "computermenuscene.h"
#include "private/computermenuscene_p.h"
#include "utils/computerdatastruct.h"
#include "controller/computercontroller.h"

#include <dfm-base/file/entry/entryfileinfo.h>
#include <dfm-base/dbusservice/global_server_defines.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/dfm_menu_defines.h>

#include "plugins/common/core/dfmplugin-menu/menu_eventinterface_helper.h"

#include <QMenu>
#include <QRegularExpression>

DFMBASE_USE_NAMESPACE
DPCOMPUTER_USE_NAMESPACE

AbstractMenuScene *ComputerMenuCreator::create()
{
    return new ComputerMenuScene();
}

ComputerMenuScene::ComputerMenuScene(QObject *parent)
    : AbstractMenuScene(parent), d(new ComputerMenuScenePrivate(this))
{
}

ComputerMenuScene::~ComputerMenuScene()
{
}

QString ComputerMenuScene::name() const
{
    return ComputerMenuCreator::name();
}

bool ComputerMenuScene::initialize(const QVariantHash &params)
{
    d->windowId = params.value(MenuParamKey::kWindowId).toULongLong();
    d->selectFiles = params.value(MenuParamKey::kSelectFiles).value<QList<QUrl>>();
    d->currentDir = params.value(MenuParamKey::kCurrentDir).toUrl();
    d->isEmptyArea = false;

    if (d->selectFiles.count() == 0)
        return false;

    d->info.reset(new EntryFileInfo(d->selectFiles.first()));

    auto subScenes = subscene();
    if (auto filterScene = dfmplugin_menu_util::menuSceneCreateScene("DConfigMenuFilter"))
        subScenes << filterScene;

    if (auto actionIconManagerScene = dfmplugin_menu_util::menuSceneCreateScene("ActionIconManager"))
        subScenes << actionIconManagerScene;

    setSubscene(subScenes);
    return AbstractMenuScene::initialize(params);
}

bool ComputerMenuScene::create(QMenu *parent)
{
    if (!parent)
        return false;

    using namespace ContextMenuAction;

    d->triggerFromSidebar = parent->property(kActionTriggeredFromSidebar).toBool();

    auto addAct = [&](const QString &key) {
        auto act = parent->addAction(d->predicateName[key]);
        d->predicateAction[key] = act;
        act->setProperty(ActionPropertyKey::kActionID, key);
    };
    auto addSep = [&] { parent->addSeparator(); };

    addAct(kOpenInNewWin);
    addAct(kOpenInNewTab);
    addAct(kOpen);
    addSep();

    addAct(kMount);
    addAct(kUnmount);
    addAct(kRename);
    addAct(kFormat);
    addAct(kErase);
    addAct(kEject);
    addAct(kSafelyRemove);
    addAct(kLogoutAndForget);
    addSep();

    addAct(kProperty);

    return AbstractMenuScene::create(parent);
}

void ComputerMenuScene::updateState(QMenu *parent)
{
    if (d->selectFiles.count() == 0)
        return;

    using namespace ContextMenuAction;
    using namespace GlobalServerDefines;
    if (!d->info)
        return;

    QStringList disabled, keeped;
    switch (d->info->order()) {
    case AbstractEntryFileEntity::kOrderUserDir:
        keeped = QStringList { kOpenInNewWin, kOpenInNewTab, kProperty };
        break;

    case AbstractEntryFileEntity::kOrderSysDiskRoot:
    case AbstractEntryFileEntity::kOrderSysDiskData:
    case AbstractEntryFileEntity::kOrderSysDisks:
        keeped = QStringList { kOpenInNewWin, kOpenInNewTab, kRename, kProperty };
        if (!d->info->renamable())
            disabled << kRename;
        break;

    case AbstractEntryFileEntity::kOrderRemovableDisks: {
        keeped = QStringList { kOpenInNewWin, kOpenInNewTab, kSafelyRemove, kProperty, kRename, kFormat };
        keeped << (d->info->targetUrl().isValid() ? QStringList { kUnmount } : QStringList { kMount });

        if (!d->info->renamable())
            disabled << kRename;
    } break;

    case AbstractEntryFileEntity::kOrderOptical: {
        keeped = QStringList { kOpenInNewWin, kOpenInNewTab, kSafelyRemove, kEject, kProperty };
        bool optical = d->info->extraProperty(DeviceProperty::kOptical).toBool();
        bool erasable = d->info->extraProperty(DeviceProperty::kMedia).toString().contains(QRegularExpression("_r(w|e)"));

        keeped << (d->info->targetUrl().isValid() ? kUnmount : kMount);
        if (keeped.contains(kMount) && erasable)
            keeped << kErase;

        if (!optical) {
            disabled = keeped;
            disabled.removeAll(kEject);
        }

        if (d->info->extraProperty(DeviceProperty::kOpticalBlank).toBool())
            disabled << kErase << kMount;

        if (DeviceUtils::isWorkingOpticalDiscDev(d->info->extraProperty(DeviceProperty::kDevice).toString()))
            disabled = keeped;
    } break;

    case AbstractEntryFileEntity::kOrderSmb:
    case AbstractEntryFileEntity::kOrderFtp: {
        keeped = QStringList { kOpenInNewWin, kOpenInNewTab, kProperty };

        if (d->info->targetUrl().isValid())
            keeped << kUnmount;

        auto id = d->info->extraProperty(DeviceProperty::kId).toString();
        if (id.contains(QRegularExpression("^smb|^ftp|^sftp|^dav")) || DeviceUtils::isSamba(QUrl(id)))
            keeped << kLogoutAndForget;
    } break;

    case AbstractEntryFileEntity::kOrderMTP:
    case AbstractEntryFileEntity::kOrderGPhoto2:
    case AbstractEntryFileEntity::kOrderFiles: {
        keeped = QStringList { kOpenInNewWin, kOpenInNewTab, kUnmount, kProperty };
    } break;
    case AbstractEntryFileEntity::kOrderApps:
        keeped = QStringList { kOpen };
        break;

    default:
        break;
    }

    if (!dpfSlotChannel->push("dfmplugin_titlebar", "slot_Tab_Addable", d->windowId).toBool())
        disabled.append(kOpenInNewTab);

    // do not show 'rename' entry for loop devices.
    if (d->info->extraProperties().value(DeviceProperty::kIsLoopDevice, false).toBool())
        keeped.removeAll(kRename);

    if (!keeped.isEmpty())
        d->updateMenu(parent, disabled, keeped);
    AbstractMenuScene::updateState(parent);
}

bool ComputerMenuScene::triggered(QAction *action)
{
    using namespace ContextMenuAction;
    auto key = action->property(ActionPropertyKey::kActionID).toString();
    if (d->predicateAction.contains(key) && d->predicateAction.value(key) == action) {
        auto ins { ComputerControllerInstance };
        if (key == kOpen)
            ins->onOpenItem(0, d->info->urlOf(UrlInfoType::kUrl));
        else if (key == kOpenInNewTab)
            ins->actOpenInNewTab(d->windowId, d->info);
        else if (key == kOpenInNewWin)
            ins->actOpenInNewWindow(d->windowId, d->info);
        else if (key == kMount)
            ins->actMount(d->windowId, d->info);
        else if (key == kUnmount)
            ins->actUnmount(d->info);
        else if (key == kRename)
            ins->actRename(d->windowId, d->info, d->triggerFromSidebar);
        else if (key == kFormat)
            ins->actFormat(d->windowId, d->info);
        else if (key == kEject)
            ins->actEject(d->info->urlOf(UrlInfoType::kUrl));
        else if (key == kErase)
            ins->actErase(d->info);
        else if (key == kSafelyRemove)
            ins->actSafelyRemove(d->info);
        else if (key == kLogoutAndForget)
            ins->actLogoutAndForgetPasswd(d->info);
        else if (key == kProperty)
            ins->actProperties(d->windowId, d->info);
        else
            return false;

        return true;
    }

    return AbstractMenuScene::triggered(action);
}

AbstractMenuScene *ComputerMenuScene::scene(QAction *action) const
{
    if (action == nullptr)
        return nullptr;

    if (!d->predicateAction.key(action).isEmpty())
        return const_cast<ComputerMenuScene *>(this);

    return AbstractMenuScene::scene(action);
}

ComputerMenuScenePrivate::ComputerMenuScenePrivate(ComputerMenuScene *qq)
    : AbstractMenuScenePrivate(qq)
{
    using namespace ContextMenuAction;
    predicateName[kOpen] = trOpen();
    predicateName[kOpenInNewWin] = trOpenInNewWin();
    predicateName[kOpenInNewTab] = trOpenInNewTab();
    predicateName[kMount] = trMount();
    predicateName[kUnmount] = trUnmount();
    predicateName[kRename] = trRename();
    predicateName[kFormat] = trFormat();
    predicateName[kEject] = trEject();
    predicateName[kErase] = trErase();
    predicateName[kSafelyRemove] = trSafelyRemove();
    predicateName[kLogoutAndForget] = trLogoutAndClearSavedPasswd();
    predicateName[kProperty] = trProperties();
}

void ComputerMenuScenePrivate::updateMenu(QMenu *menu, const QStringList &disabled, const QStringList &keeps)
{
    if (!menu)
        return;

    std::for_each(disabled.cbegin(), disabled.cend(), [this](const QString &act) {
        if (predicateAction.value(act))
            predicateAction.value(act)->setEnabled(false);
    });

    auto actions = menu->actions();
    std::for_each(actions.cbegin(), actions.cend(), [&](QAction *act) {
        if (act && !act->isSeparator()) {
            auto key = act->property(ActionPropertyKey::kActionID).toString();
            if (!keeps.contains(key))
                act->setVisible(false);
        }
    });
}
