// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "private/opendirmenuscene_p.h"
#include "action_defines.h"

#include <dfm-base/dfm_menu_defines.h>
#include <dfm-base/utils/systempathutil.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/utils/sysinfoutils.h>
#include <dfm-base/utils/protocolutils.h>

#include <dfm-framework/dpf.h>

#include <QMenu>
#include <QVariant>

using namespace dfmplugin_menu;
DFMBASE_USE_NAMESPACE

AbstractMenuScene *OpenDirMenuCreator::create()
{
    return new OpenDirMenuScene();
}

OpenDirMenuScenePrivate::OpenDirMenuScenePrivate(OpenDirMenuScene *qq)
    : AbstractMenuScenePrivate(qq)
{
    predicateName[ActionID::kOpenAsAdmin] = tr("Open as administrator");
    predicateName[ActionID::kSelectAll] = tr("Select all");
    predicateName[ActionID::kOpenInNewWindow] = tr("Open in new window");
    predicateName[ActionID::kOpenInNewTab] = tr("Open in new tab");
    predicateName[ActionID::kOpenInTerminal] = tr("Open in terminal");
    predicateName[ActionID::kReverseSelect] = tr("Reverse select");
}

OpenDirMenuScene::OpenDirMenuScene(QObject *parent)
    : AbstractMenuScene(parent),
      d(new OpenDirMenuScenePrivate(this))
{
}

QString OpenDirMenuScene::name() const
{
    return OpenDirMenuCreator::name();
}

bool OpenDirMenuScene::initialize(const QVariantHash &params)
{
    d->currentDir = params.value(MenuParamKey::kCurrentDir).toUrl();
    d->selectFiles = params.value(MenuParamKey::kSelectFiles).value<QList<QUrl>>();
    if (!d->selectFiles.isEmpty())
        d->focusFile = d->selectFiles.first();
    d->onDesktop = params.value(MenuParamKey::kOnDesktop).toBool();
    d->windowId = params.value(MenuParamKey::kWindowId).toULongLong();
    d->isEmptyArea = params.value(MenuParamKey::kIsEmptyArea).toBool();

    if (!d->initializeParamsIsValid()) {
        fmWarning() << "menu scene:" << name() << " init failed." << d->selectFiles.isEmpty() << d->focusFile << d->currentDir;
        return false;
    }

    if (!d->isEmptyArea) {
        QString errString;
        d->focusFileInfo = DFMBASE_NAMESPACE::InfoFactory::create<FileInfo>(d->focusFile, Global::CreateFileInfoType::kCreateFileInfoAuto, &errString);
        if (d->focusFileInfo.isNull()) {
            fmDebug() << errString;
            return false;
        }
    }

    return AbstractMenuScene::initialize(params);
}

AbstractMenuScene *OpenDirMenuScene::scene(QAction *action) const
{
    if (action == nullptr)
        return nullptr;

    if (d->predicateAction.values().contains(action))
        return const_cast<OpenDirMenuScene *>(this);

    return AbstractMenuScene::scene(action);
}

bool OpenDirMenuScene::create(QMenu *parent)
{
    if (d->isEmptyArea)
        emptyMenu(parent);
    else
        normalMenu(parent);

    return AbstractMenuScene::create(parent);
}

void OpenDirMenuScene::updateState(QMenu *parent)
{
    if (!parent)
        return;
    AbstractMenuScene::updateState(parent);
}

bool OpenDirMenuScene::triggered(QAction *action)
{
    if (!d->predicateAction.values().contains(action))
        return false;

    auto actionId = action->property(ActionPropertyKey::kActionID).toString();

    // open in new window
    if (actionId == ActionID::kOpenInNewWindow) {
        QUrl cdUrl = d->focusFile;
        FileInfoPointer infoPtr = InfoFactory::create<FileInfo>(cdUrl);
        if (infoPtr && infoPtr->isAttributes(OptInfoType::kIsSymLink))
            cdUrl = QUrl::fromLocalFile(infoPtr->pathOf(PathInfoType::kSymLinkTarget));
        dpfSignalDispatcher->publish(GlobalEventType::kOpenNewWindow, cdUrl);
        return true;
    }

    // Open in terminal
    if (actionId == ActionID::kOpenInTerminal) {
        QList<QUrl> urls;
        if (d->isEmptyArea)
            urls << d->currentDir;
        else
            urls << d->focusFile;

        dpfSignalDispatcher->publish(GlobalEventType::kOpenInTerminal, d->windowId, urls);
        return true;
    }

    // open as admin
    if (actionId == ActionID::kOpenAsAdmin) {
        dpfSignalDispatcher->publish(GlobalEventType::kOpenAsAdmin, d->isEmptyArea ? d->currentDir : d->focusFile);
        return true;
    }

    // select all
    if (actionId == dfmplugin_menu::ActionID::kSelectAll) {
        dpfSlotChannel->push("dfmplugin_workspace", "slot_View_SelectAll", d->windowId);
        return true;
    }

    // TODO(Lee or others):
    return AbstractMenuScene::triggered(action);
}

void OpenDirMenuScene::emptyMenu(QMenu *parent)
{
    openAsAdminAction(parent);
    QAction *tempAction = parent->addAction(d->predicateName.value(ActionID::kSelectAll));
    d->predicateAction[ActionID::kSelectAll] = tempAction;
    tempAction->setProperty(ActionPropertyKey::kActionID, ActionID::kSelectAll);

    tempAction = parent->addAction(d->predicateName.value(ActionID::kOpenInTerminal));
    d->predicateAction[ActionID::kOpenInTerminal] = tempAction;
    tempAction->setProperty(ActionPropertyKey::kActionID, ActionID::kOpenInTerminal);
}

void OpenDirMenuScene::normalMenu(QMenu *parent)
{
    if (d->selectFiles.count() == 1 && d->focusFileInfo->isAttributes(OptInfoType::kIsDir)) {
        QAction *tempAction = parent->addAction(d->predicateName.value(ActionID::kOpenInNewWindow));
        d->predicateAction[ActionID::kOpenInNewWindow] = tempAction;
        tempAction->setProperty(ActionPropertyKey::kActionID, ActionID::kOpenInNewWindow);

        tempAction = parent->addAction(d->predicateName.value(ActionID::kOpenInNewTab));
        d->predicateAction[ActionID::kOpenInNewTab] = tempAction;
        tempAction->setProperty(ActionPropertyKey::kActionID, ActionID::kOpenInNewTab);

        tempAction = parent->addAction(d->predicateName.value(ActionID::kOpenInTerminal));
        d->predicateAction[ActionID::kOpenInTerminal] = tempAction;
        tempAction->setProperty(ActionPropertyKey::kActionID, ActionID::kOpenInTerminal);

        openAsAdminAction(parent);
    }

    QAction *tempAction = parent->addAction(d->predicateName.value(ActionID::kReverseSelect));
    d->predicateAction[ActionID::kReverseSelect] = tempAction;
    tempAction->setProperty(ActionPropertyKey::kActionID, ActionID::kReverseSelect);
}

void OpenDirMenuScene::openAsAdminAction(QMenu *parent)
{
    // root user, server version user and non developer mode do not need to be opened as an administrator
    if (!SysInfoUtils::isDeveloperModeEnabled())
        return;
    if (SysInfoUtils::isRootUser() || SysInfoUtils::isServerSys())
        return;

    if (ProtocolUtils::isRemoteFile(d->currentDir)) {   // gvfs mounts and new smb mounts
        fmDebug() << "openAsAdmin is not added cause GVFS file: " << d->currentDir;
        return;
    }

    QAction *tempAction = parent->addAction(d->predicateName.value(ActionID::kOpenAsAdmin));
    d->predicateAction[ActionID::kOpenAsAdmin] = tempAction;
    tempAction->setProperty(ActionPropertyKey::kActionID, ActionID::kOpenAsAdmin);
}
