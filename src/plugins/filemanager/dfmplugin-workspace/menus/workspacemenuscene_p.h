/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuzhangjian<liqianga@uniontech.com>
 *
 * Maintainer: liuzhangjian<liqianga@uniontech.com>
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
#ifndef WORKSPACEMENUSCENE_P_H
#define WORKSPACEMENUSCENE_P_H

#include "workspacemenuscene.h"
#include "interfaces/private/abstractmenuscene_p.h"
#include "workspacemenu_defines.h"

#include "services/common/dfm_common_service_global.h"
#include "services/common/menu/menuservice.h"
#include "services/common/menu/menu_defines.h"
#include <plugins/common/dfmplugin-menu/menuScene/action_defines.h>

DPWORKSPACE_BEGIN_NAMESPACE

class FileView;
class WorkspaceMenuScenePrivate : public DFMBASE_NAMESPACE::AbstractMenuScenePrivate
{
public:
    explicit WorkspaceMenuScenePrivate(WorkspaceMenuScene *qq);

    void sortMenuAction(QMenu *menu, const QStringList &sortRule);

    inline QStringList emptyMenuActionRule()
    {
        static QStringList actionRule {
            dfmplugin_menu::ActionID::kNewFolder,
            dfmplugin_menu::ActionID::kNewDoc,
            ActionID::kSeparator,
            ActionID::kDisplayAs,
            ActionID::kSortBy,
            dfmplugin_menu::ActionID::kOpenAsAdmin,
            dfmplugin_menu::ActionID::kOpenInTerminal,
            ActionID::kSeparator,
            dfmplugin_menu::ActionID::kPaste,
            dfmplugin_menu::ActionID::kSelectAll,
        };

        return actionRule;
    }

public:
    FileView *view = nullptr;
    DSC_NAMESPACE::MenuService *menuServer = nullptr;

private:
    WorkspaceMenuScene *q;
};

DPWORKSPACE_END_NAMESPACE

#endif   // WORKSPACEMENUSCENE_P_H
