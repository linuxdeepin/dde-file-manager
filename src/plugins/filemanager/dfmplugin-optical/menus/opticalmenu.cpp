/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#include "opticalmenu.h"
#include "utils/opticalhelper.h"

#include "dfm-base/utils/actiontypemanager.h"

DPOPTICAL_BEGIN_NAMESPACE
namespace OpticalScene {
const char *const kOpticalMenu = "optical-menu";
}   // namespace OpticalScene
DPOPTICAL_END_NAMESPACE

DPOPTICAL_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DSB_FM_USE_NAMESPACE

OpticalMenu::OpticalMenu(QObject *parent)
    : AbstractMenu(parent)
{
}

QMenu *OpticalMenu::build(QWidget *parent,
                          AbstractMenu::MenuMode mode,
                          const QUrl &rootUrl,
                          const QUrl &focusUrl,
                          const QList<QUrl> &selected,
                          QVariant customData)
{
    Q_UNUSED(customData)

    curFocusUrl = focusUrl;
    curSelectedUrls = selected;

    // get defualt workspace menu
    QMenu *menu = OpticalHelper::menuServIns()->createMenu(parent, Workspace::MenuScene::kWorkspaceMenu, mode,
                                                           rootUrl, focusUrl, selected);
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

void OpticalMenu::assemblesEmptyAreaActions(QMenu *menu)
{
    static QVector<ActionType> typeList { ActionType::kActDisplayAs, ActionType::kActSortBy, ActionType::kActPaste, ActionType::kActOpenAsAdmin,
                                          ActionType::kActOpenInTerminal, ActionType::kActSelectAll, ActionType::kActProperty, ActionType::kActSeparator };
    filterActions(menu, typeList);
}

void OpticalMenu::assemblesNormalActions(QMenu *menu)
{
    QVector<ActionType> list;
    static QVector<ActionType> defaultList { ActionType::kActCut, ActionType::kActRename,
                                             ActionType::kActSendToRemovableDisk, ActionType::kActStageFileForBurning };

    if (OpticalHelper::burnIsOnDisc(curFocusUrl)) {
        list.push_back(ActionType::kActDelete);
        list.push_back(ActionType::kActCompleteDeletion);
    } else {
        list.push_back(ActionType::kActOpenInTerminal);
        list.push_back(ActionType::kActOpenAsAdmin);
        list.push_back(ActionType::kActCopy);
        list.push_back(ActionType::kActCompress);
        list.push_back(ActionType::kActCreateSymlink);
        list.push_back(ActionType::kActSendToDesktop);
    }
    list += defaultList;
    filterActions(menu, list, true);
}

void OpticalMenu::filterActions(QMenu *menu, const QVector<ActionType> &typeList, bool reverse)
{
    if (!menu)
        return;

    QStringList actionNames;
    for (const ActionType type : typeList) {
        auto actionData = ActionTypeManager::instance().actionDataContainerByType(type);
        actionNames << actionData.name();
    }

    for (const auto &action : menu->actions()) {
        if (reverse) {
            if (actionNames.contains(action->text()))
                menu->removeAction(action);
        } else {
            if (!actionNames.contains(action->text()))
                menu->removeAction(action);
        }
    }
}
