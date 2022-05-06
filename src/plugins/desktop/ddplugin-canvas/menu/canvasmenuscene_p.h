/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liqiang<liqianga@uniontech.com>
 *
 * Maintainer: liqiang<liqianga@uniontech.com>
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
#ifndef CANVASMENUSCENE_P_H
#define CANVASMENUSCENE_P_H
#include "canvasmenuscene.h"
#include "interfaces/private/abstractmenuscene_p.h"
#include "canvasmenu_defines.h"

#include <services/common/dfm_common_service_global.h>
#include <services/common/menu/menuservice.h>
#include "services/common/menu/menu_defines.h"
#include <plugins/common/dfmplugin-menu/menuscene/action_defines.h>

#include <QMap>
#include <QMultiHash>

DSC_USE_NAMESPACE
DDP_CANVAS_BEGIN_NAMESPACE

class CanvasView;
class CanvasMenuScenePrivate : public dfmbase::AbstractMenuScenePrivate
{
public:
    explicit CanvasMenuScenePrivate(CanvasMenuScene *qq);

    void filterDisableAction(QMenu *menu);

    void sortMenuAction(QMenu *menu, const QStringList &sortRule);
    inline QStringList emptyMenuActionRules()
    {
        static QStringList actionRule {
            dfmplugin_menu::ActionID::kNewFolder,
            dfmplugin_menu::ActionID::kNewDoc,
            ActionID::kSortBy,
            ActionID::kIconSize,
            ActionID::kAutoArrange,
            dfmplugin_menu::ActionID::kPaste,
            dfmplugin_menu::ActionID::kSelectAll,
            dfmplugin_menu::ActionID::kOpenInTerminal,
            dfmplugin_menu::ActionID::kSeparator,
            ActionID::kDisplaySettings,
            ActionID::kWallpaperSettings,
        };
        return actionRule;
    }
    inline QStringList normalMenuActionRules()
    {
        static QStringList actionRule {
            dfmplugin_menu::ActionID::kOpen,
            dfmplugin_menu::ActionID::kOpenWith,
            dfmplugin_menu::ActionID::kSeparator,
            dfmplugin_menu::ActionID::kOpenAsAdmin,
            dfmplugin_menu::ActionID::kSeparator,
            dfmplugin_menu::ActionID::kEmptyTrash,
            dfmplugin_menu::ActionID::kCut,
            dfmplugin_menu::ActionID::kCopy,
            dfmplugin_menu::ActionID::kRename,
            dfmplugin_menu::ActionID::kDelete,
            dfmplugin_menu::ActionID::kSeparator,
            dfmplugin_menu::ActionID::kCreateSymlink,
            dfmplugin_menu::ActionID::kSeparator,
            dfmplugin_menu::ActionID::kOpenInTerminal
        };
        return actionRule;
    }

public:
    QMap<QAction *, int> iconSizeAction;
    QPoint gridPos;

    CanvasView *view { nullptr };
    MenuService *menuServer = nullptr;

    QMultiHash<QString, QString> emptyDisableActions;
    QMultiHash<QString, QString> normalDisableActions;

private:
    CanvasMenuScene *q;
};

DDP_CANVAS_END_NAMESPACE
#endif   // CANVASMENUSCENE_P_H
