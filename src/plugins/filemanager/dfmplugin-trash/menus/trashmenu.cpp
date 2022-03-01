/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuyangming<liuyangming@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "trashmenu.h"
#include "utils/trashhelper.h"
#include "utils/trashfilehelper.h"

#include "services/filemanager/workspace/workspace_defines.h"
#include "services/filemanager/workspace/workspaceservice.h"
#include "services/common/menu/menuservice.h"

#include "dfm-base/dfm_actiontype_defines.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/interfaces/abstractfileactions.h"
#include "dfm-base/utils/actiontypemanager.h"
#include "dfm-framework/framework.h"

#include <QMenu>
#include <QAction>

DPTRASH_BEGIN_NAMESPACE
namespace TrashScene {
extern const char *const kTrashMenu = "trash-menu";
}   // namespace MenuScene
DPTRASH_END_NAMESPACE

DSB_FM_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DPTRASH_USE_NAMESPACE

TrashMenu::TrashMenu(QObject *parent)
    : AbstractMenu(parent)
{
}

QMenu *TrashMenu::build(QWidget *parent,
                        AbstractMenu::MenuMode mode,
                        const QUrl &rootUrl,
                        const QUrl &foucsUrl,
                        const QList<QUrl> &selected,
                        QVariant customData)
{
    Q_UNUSED(customData)
    this->rootUrl = rootUrl;
    this->foucsUrl = foucsUrl;
    this->selectedUrls = selected;
    QMenu *menu = createMenu(parent,
                             Workspace::MenuScene::kWorkspaceMenu,
                             mode,
                             rootUrl,
                             foucsUrl,
                             selected,
                             false,
                             ExtensionType::kNoExtensionAction,
                             customData);

    switch (mode) {
    case AbstractMenu::MenuMode::kEmpty:
        assemblesEmptyAreaActions(menu);
        break;

    case AbstractMenu::MenuMode::kNormal:
        assemblesNormalActions(menu);
        break;
    }
    return menu;
}

void TrashMenu::actionBusiness(QAction *act)
{
    auto actType = act->data().toInt();

    switch (actType) {
    case TrashActionType::kRestore:
        restore();
        break;
    case TrashActionType::kRestoreAll:
        restoreAll();
        break;
    case TrashActionType::kEmptyTrash:
        emptyTrash();
        break;

    default:
        break;
    }
}

void TrashMenu::assemblesEmptyAreaActions(QMenu *menu)
{
    filterEmptyActions(menu);
    auto isDisabled = rootUrl != TrashHelper::rootUrl();
    QAction *restoreAllAct = createAction(TrashActionType::kRestoreAll, QObject::tr("Restore all"), isDisabled);

    QAction *emptyTrashAct = createAction(TrashActionType::kEmptyTrash, QObject::tr("Empty trash"), isDisabled);
    menu->insertSeparator(menu->actions().first());
    menu->insertAction(menu->actions().first(), emptyTrashAct);
    menu->insertAction(menu->actions().first(), restoreAllAct);
}

void TrashMenu::assemblesNormalActions(QMenu *menu)
{
    filterNormalActions(menu);

    auto isDisabled = rootUrl != TrashHelper::rootUrl();

    if (isDisabled) {
        QStringList disableText;
        QVector<ActionType> disableTypeList;
        disableTypeList << ActionType::kActDelete << ActionType::kActCut;
        for (const ActionType type : disableTypeList) {
            auto actionData = ActionTypeManager::instance().actionDataContainerByType(type);
            disableText << actionData.name();
        }

        for (const auto &action : menu->actions()) {
            if (disableText.contains(action->text()))
                action->setDisabled(true);
        }
    }
    QAction *restoreAct = createAction(TrashActionType::kRestore, QObject::tr("Restore"), isDisabled);

    auto cutActionData = ActionTypeManager::instance().actionDataContainerByType(ActionType::kActCut);

    QAction *cutAction = nullptr;
    for (const auto &action : menu->actions()) {
        if (action->text() == cutActionData.name()) {
            cutAction = action;
            break;
        }
    }
    if (cutAction)
        menu->insertAction(cutAction, restoreAct);
}

void TrashMenu::filterNormalActions(QMenu *menu)
{

    QVector<ActionType> typeList;
    bool isMultipleSelected = selectedUrls.count() > 1;

    const AbstractFileInfoPointer &fileInfo = InfoFactory::create<AbstractFileInfo>(foucsUrl);
    bool isFoucsDir = fileInfo && fileInfo->isDir();
    bool isMultipleFileSelected = false;
    if (isMultipleSelected) {
        for (const auto &url : selectedUrls) {
            const AbstractFileInfoPointer &fileInfo = InfoFactory::create<AbstractFileInfo>(url);
            if (fileInfo->isFile())
                isMultipleFileSelected = true;
        }
    }

    if (!isMultipleFileSelected && isFoucsDir) {
        typeList << ActionType::kActOpen << kActOpenInNewWindow;
    }
    if (isMultipleFileSelected && isFoucsDir) {
        typeList << ActionType::kActOpen;
    }
    typeList << ActionType::kActDelete << ActionType::kActCopy << ActionType::kActCut << ActionType::kActProperty << ActionType::kActSeparator;
    filterActions(menu, typeList);
}

void TrashMenu::filterEmptyActions(QMenu *menu)
{
    QVector<ActionType> typeList { ActionType::kActDisplayAs, ActionType::kActSortBy, ActionType::kActProperty, ActionType::kActSeparator };
    filterActions(menu, typeList);
}

void TrashMenu::filterActions(QMenu *menu, const QVector<ActionType> &typeList)
{
    if (!menu)
        return;

    QStringList actionNames;
    for (const ActionType type : typeList) {
        auto actionData = ActionTypeManager::instance().actionDataContainerByType(type);
        actionNames << actionData.name();
    }

    for (const auto &action : menu->actions()) {
        if (!actionNames.contains(action->text()))
            menu->removeAction(action);
    }
}

void TrashMenu::emptyTrash()
{
    TrashHelper::emptyTrash();
}

void TrashMenu::restore()
{
    TrashFileHelper::restoreFromTrashHandle(0, this->selectedUrls, AbstractJobHandler::JobFlag::kRevocation);
}

void TrashMenu::restoreAll()
{
    TrashFileHelper::restoreFromTrashHandle(0, { this->rootUrl }, AbstractJobHandler::JobFlag::kRevocation);
}

QAction *TrashMenu::createAction(const TrashMenu::TrashActionType type, const QString &text, bool isDisabled)
{

    QAction *action = new QAction;
    action->setData(int(type));
    action->setText(text);
    action->setDisabled(isDisabled);
    return action;
}

QMenu *TrashMenu::createMenu(QWidget *parent, const QString &scene, AbstractMenu::MenuMode mode, const QUrl &rootUrl, const QUrl &foucsUrl, const QList<QUrl> selected, bool onDesktop, ExtensionType flags, QVariant customData)
{

    auto &ctx = dpfInstance.serviceContext();
    static std::once_flag onceFlag;
    std::call_once(onceFlag, [&ctx]() {
        if (!ctx.load(DSC_NAMESPACE::MenuService::name()))
            abort();
    });

    auto menuService = ctx.service<DSC_NAMESPACE::MenuService>(DSC_NAMESPACE::MenuService::name());

    return menuService->createMenu(parent,
                                   scene,
                                   mode,
                                   rootUrl,
                                   foucsUrl,
                                   selected,
                                   onDesktop,
                                   flags,
                                   customData);
}
