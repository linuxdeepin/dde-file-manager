/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuzhangjian<liuzhangjian@uniontech.com>
 *
 * Maintainer: liuzhangjian<liuzhangjian@uniontech.com>
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
#include "searchmenu.h"
#include "utils/searchhelper.h"

#include "services/filemanager/workspace/workspace_defines.h"
#include "services/filemanager/workspace/workspaceservice.h"
#include "services/common/menu/menuservice.h"

#include "dfm-base/dfm_actiontype_defines.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/interfaces/abstractfileactions.h"
#include "dfm-base/utils/actiontypemanager.h"
#include "dfm-framework/framework.h"

#include <DDesktopServices>

DWIDGET_USE_NAMESPACE
DSB_FM_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DPSEARCH_BEGIN_NAMESPACE

namespace SearchScene {
const char *const kSearchMenu = "search-menu";
}   // namespace SearchScene

SearchMenu::SearchMenu(QObject *parent)
    : AbstractMenu(parent)
{
}

QMenu *SearchMenu::build(QWidget *parent,
                         AbstractMenu::MenuMode mode,
                         const QUrl &rootUrl,
                         const QUrl &foucsUrl,
                         const QList<QUrl> &selected,
                         QVariant customData)
{
    Q_UNUSED(customData)

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
    case AbstractMenu::kEmpty:
        assemblesEmptyAreaActions(menu);
        break;

    case AbstractMenu::kNormal:
        assemblesNormalActions(menu);
        break;
    }
    return menu;
}

void SearchMenu::actionBusiness(QAction *act)
{
    auto actType = act->data().toInt();

    switch (actType) {
    case SearchActionType::kOpenFileLocation:
        openFileLocation();
        break;
    default:
        break;
    }
}

void SearchMenu::assemblesEmptyAreaActions(QMenu *menu)
{
    QVector<ActionType> typeList { ActionType::kActDisplayAs, ActionType::kActSortBy, ActionType::kActSelectAll, ActionType::kActSeparator };
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

void SearchMenu::assemblesNormalActions(QMenu *menu)
{
    QAction *locationAct = new QAction(QObject::tr("Open file location"), menu);
    locationAct->setData(int(kOpenFileLocation));

    auto openActionData = ActionTypeManager::instance().actionDataContainerByType(ActionType::kActOpen);
    QAction *openAction = nullptr;
    for (const auto &action : menu->actions()) {
        if (action->text() == openActionData.name()) {
            openAction = action;
            break;
        }
    }
    if (openAction)
        // todo (liuzhangjian) insert before open as action
        menu->insertAction(openAction, locationAct);
}

void SearchMenu::openFileLocation()
{
    // todo (liuzhangjian)
    // root user can not use 'DDesktopServices::showFileItem'

    QList<QUrl> realUrls;
    for (const QUrl &url : selectedUrls) {
        realUrls << SearchHelper::searchedFileUrl(url);
    }

    DDesktopServices::showFileItems(realUrls);
}

QMenu *SearchMenu::createMenu(QWidget *parent,
                              const QString &scene,
                              AbstractMenu::MenuMode mode,
                              const QUrl &rootUrl,
                              const QUrl &foucsUrl,
                              const QList<QUrl> selected,
                              bool onDesktop,
                              ExtensionType flags,
                              QVariant customData)
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

DPSEARCH_END_NAMESPACE
