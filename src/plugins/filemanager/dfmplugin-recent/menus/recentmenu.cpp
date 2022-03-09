/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
 *             liuyangming<liuyangming@uniontech.com>
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
#include "recentmenu.h"
#include "utils/recentmanager.h"
#include "utils/recentfileshelper.h"

#include "services/filemanager/workspace/workspace_defines.h"
#include "services/filemanager/workspace/workspaceservice.h"
#include "services/common/menu/menuservice.h"

#include "dfm-base/base/schemefactory.h"
#include "dfm-base/interfaces/abstractfileactions.h"
#include "dfm-base/utils/actiontypemanager.h"
#include "dfm-framework/framework.h"

#include <DDialog>
#include <DRecentManager>

#include <QMenu>
#include <QAction>

DPRECENT_BEGIN_NAMESPACE
namespace RecentScene {
const char *const kRecentMenu = "recent-menu";
}   // namespace MenuScene
DPRECENT_END_NAMESPACE

DSB_FM_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DCORE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
DPRECENT_USE_NAMESPACE

RecentMenu::RecentMenu(QObject *parent)
    : AbstractMenu(parent)
{
}

QMenu *RecentMenu::build(QWidget *parent,
                         AbstractMenu::MenuMode mode,
                         const QUrl &rootUrl,
                         const QUrl &focusUrl,
                         const QList<QUrl> &selected,
                         QVariant customData)
{
    Q_UNUSED(customData)
    this->rootUrl = rootUrl;
    this->focusUrl = focusUrl;
    this->selectedUrls = selected;
    QMenu *menu = createMenu(parent,
                             Workspace::MenuScene::kWorkspaceMenu,
                             mode,
                             rootUrl,
                             focusUrl,
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

void RecentMenu::removeRecent()
{
    RecentFilesHelper::removeRecent(selectedUrls);
}

void RecentMenu::actionBusiness(QAction *act)
{
    auto actType = static_cast<RecentActionType>(act->data().toInt());

    switch (actType) {
    case RecentActionType::kRemove:
        removeRecent();
        break;

    default:
        break;
    }
}

void RecentMenu::assemblesEmptyAreaActions(QMenu *menu)
{
    filterEmptyActions(menu);
}

void RecentMenu::assemblesNormalActions(QMenu *menu)
{
    filterNormalActions(menu);

    QAction *removeAct = createAction(RecentActionType::kRemove, QObject::tr("Remove"));

    auto beforActionData = ActionTypeManager::instance().actionDataContainerByType(ActionType::kActSendToDesktop);

    QAction *beforAction = nullptr;
    for (const auto &action : menu->actions()) {
        if (action->text() == beforActionData.name()) {
            beforAction = action;
            break;
        }
    }
    if (beforAction)
        menu->insertAction(beforAction, removeAct);
}

void RecentMenu::filterNormalActions(QMenu *menu)
{
    QVector<ActionType> typeList;

    typeList << ActionType::kActOpen << ActionType::kActOpenWith << ActionType::kActCopy << ActionType::kActSendToDesktop << ActionType::kActCreateSymlink << ActionType::kActProperty << ActionType::kActSeparator;
    filterActions(menu, typeList);
}

void RecentMenu::filterEmptyActions(QMenu *menu)
{
    QVector<ActionType> typeList { ActionType::kActDisplayAs, ActionType::kActSortBy, ActionType::kActSelectAll, ActionType::kActSeparator };
    filterActions(menu, typeList);
}

void RecentMenu::filterActions(QMenu *menu, const QVector<ActionType> &typeList)
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

QAction *RecentMenu::createAction(const RecentMenu::RecentActionType type, const QString &text, bool isDisabled)
{

    QAction *action = new QAction;
    action->setData(int(type));
    action->setText(text);
    action->setDisabled(isDisabled);
    return action;
}

QMenu *RecentMenu::createMenu(QWidget *parent, const QString &scene, AbstractMenu::MenuMode mode, const QUrl &rootUrl, const QUrl &focusUrl, const QList<QUrl> selected, bool onDesktop, ExtensionType flags, QVariant customData)
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
                                   focusUrl,
                                   selected,
                                   onDesktop,
                                   flags,
                                   customData);
}
