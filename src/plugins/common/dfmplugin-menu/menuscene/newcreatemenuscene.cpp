// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "newcreatemenuscene.h"
#include "private/newcreatemenuscene_p.h"
#include "action_defines.h"
#include "templatemenuscene/templatemenuscene.h"

#include "plugins/common/dfmplugin-menu/menu_eventinterface_helper.h"

#include <dfm-base/dfm_menu_defines.h>
#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/base/schemefactory.h>

#include <QMenu>
#include <QVariant>

using namespace dfmplugin_menu;
DFMBASE_USE_NAMESPACE

static const char *const kTemplateMenuSceneName = "TemplateMenu";

AbstractMenuScene *NewCreateMenuCreator::create()
{
    return new NewCreateMenuScene();
}

NewCreateMenuScenePrivate::NewCreateMenuScenePrivate(NewCreateMenuScene *qq)
    : AbstractMenuScenePrivate(qq)
{
    predicateName[ActionID::kNewFolder] = tr("New folder");
    predicateName[ActionID::kNewDoc] = tr("New document");
    predicateName[ActionID::kNewOfficeText] = tr("Office Text");
    predicateName[ActionID::kNewSpreadsheets] = tr("Spreadsheets");
    predicateName[ActionID::kNewPresentation] = tr("Presentation");
    predicateName[ActionID::kNewPlainText] = tr("Plain Text");
}

NewCreateMenuScene::NewCreateMenuScene(QObject *parent)
    : AbstractMenuScene(parent),
      d(new NewCreateMenuScenePrivate(this))
{
}

QString NewCreateMenuScene::name() const
{
    return NewCreateMenuCreator::name();
}

bool NewCreateMenuScene::initialize(const QVariantHash &params)
{
    d->currentDir = params.value(MenuParamKey::kCurrentDir).toUrl();
    d->onDesktop = params.value(MenuParamKey::kOnDesktop).toBool();
    d->windowId = params.value(MenuParamKey::kWindowId).toULongLong();

    if (!d->currentDir.isValid())
        return false;

    QList<AbstractMenuScene *> currentScene;

    if (auto templateScene = dfmplugin_menu_util::menuSceneCreateScene(kTemplateMenuSceneName))
        currentScene.append(templateScene);

    // the scene added by binding must be initializeed after 'defalut scene'.
    currentScene.append(subScene);
    setSubscene(currentScene);

    return AbstractMenuScene::initialize(params);
}

AbstractMenuScene *NewCreateMenuScene::scene(QAction *action) const
{
    if (action == nullptr)
        return nullptr;

    if (d->predicateAction.values().contains(action))
        return const_cast<NewCreateMenuScene *>(this);

    return AbstractMenuScene::scene(action);
}

bool NewCreateMenuScene::create(QMenu *parent)
{
    if (!parent)
        return false;

    QAction *tempAction = parent->addAction(d->predicateName.value(ActionID::kNewFolder));
    d->predicateAction[ActionID::kNewFolder] = tempAction;
    // set action id
    tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kNewFolder));

    tempAction = parent->addAction(d->predicateName.value(ActionID::kNewDoc));
    d->predicateAction[ActionID::kNewDoc] = tempAction;
    tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kNewDoc));

    QMenu *newDocSubMenu = new QMenu(parent);
    tempAction->setMenu(newDocSubMenu);

    tempAction = newDocSubMenu->addAction(d->predicateName.value(ActionID::kNewOfficeText));
    d->predicateAction[ActionID::kNewOfficeText] = tempAction;
    tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kNewOfficeText));

    tempAction = newDocSubMenu->addAction(d->predicateName.value(ActionID::kNewSpreadsheets));
    d->predicateAction[ActionID::kNewSpreadsheets] = tempAction;
    tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kNewSpreadsheets));

    tempAction = newDocSubMenu->addAction(d->predicateName.value(ActionID::kNewPresentation));
    d->predicateAction[ActionID::kNewPresentation] = tempAction;
    tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kNewPresentation));

    tempAction = newDocSubMenu->addAction(d->predicateName.value(ActionID::kNewPlainText));
    d->predicateAction[ActionID::kNewPlainText] = tempAction;
    tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kNewPlainText));

    return AbstractMenuScene::create(parent);
}

void NewCreateMenuScene::updateState(QMenu *parent)
{
    if (!parent)
        return;

    auto curDirInfo = InfoFactory::create<FileInfo>(d->currentDir);
    if (!curDirInfo)
        return;
    if (!curDirInfo->isAttributes(OptInfoType::kIsWritable)) {
        auto actions = parent->actions();
        for (auto act : actions) {
            const auto &actId = act->property(ActionPropertyKey::kActionID);
            if (ActionID::kNewFolder == actId) {
                act->setDisabled(true);
            } else if (ActionID::kNewDoc == actId) {
                act->setDisabled(true);
            }
        }
    }

    AbstractMenuScene::updateState(parent);
}

bool NewCreateMenuScene::triggered(QAction *action)
{
    auto actionId = action->property(ActionPropertyKey::kActionID).toString();
    if (d->predicateAction.values().contains(action)) {
        if (actionId == dfmplugin_menu::ActionID::kNewFolder) {
            dpfSignalDispatcher->publish(GlobalEventType::kMkdir, d->windowId, d->currentDir);
        } else if (actionId == dfmplugin_menu::ActionID::kNewOfficeText) {
            dpfSignalDispatcher->publish(GlobalEventType::kTouchFile, d->windowId, d->currentDir,
                                         DFMBASE_NAMESPACE::Global::CreateFileType::kCreateFileTypeWord, QString());
        } else if (actionId == dfmplugin_menu::ActionID::kNewSpreadsheets) {
            dpfSignalDispatcher->publish(GlobalEventType::kTouchFile, d->windowId, d->currentDir,
                                         DFMBASE_NAMESPACE::Global::CreateFileType::kCreateFileTypeExcel, QString());
        } else if (actionId == dfmplugin_menu::ActionID::kNewPresentation) {
            dpfSignalDispatcher->publish(GlobalEventType::kTouchFile, d->windowId, d->currentDir,
                                         DFMBASE_NAMESPACE::Global::CreateFileType::kCreateFileTypePowerpoint, QString());
        } else if (actionId == dfmplugin_menu::ActionID::kNewPlainText) {
            dpfSignalDispatcher->publish(GlobalEventType::kTouchFile, d->windowId, d->currentDir,
                                         DFMBASE_NAMESPACE::Global::CreateFileType::kCreateFileTypeText, QString());
        }
        return true;
    }
    return AbstractMenuScene::triggered(action);
}
