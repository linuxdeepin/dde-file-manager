// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CANVASMENUSCENE_P_H
#define CANVASMENUSCENE_P_H
#include "canvasmenuscene.h"
#include "canvasmenu_defines.h"

#include "plugins/common/core/dfmplugin-menu/menuscene/action_defines.h"

#include <dfm-base/dfm_menu_defines.h>
#include <dfm-base/interfaces/private/abstractmenuscene_p.h>

#include <QMap>
#include <QMultiHash>

namespace ddplugin_canvas {

class CanvasView;
class CanvasMenuScenePrivate : public dfmbase::AbstractMenuScenePrivate
{
    Q_OBJECT
public:
    explicit CanvasMenuScenePrivate(CanvasMenuScene *qq);

    void filterDisableAction(QMenu *menu);

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
            ActionID::kRefresh,
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
            dfmplugin_menu::ActionID::kSetAsWallpaper,
            dfmplugin_menu::ActionID::kSeparator,
            dfmplugin_menu::ActionID::kOpenInTerminal
        };
        return actionRule;
    }

public:
    QMap<QAction *, int> iconSizeAction;
    QPoint gridPos;

    CanvasView *view { nullptr };

    QMultiHash<QString, QString> emptyDisableActions;
    QMultiHash<QString, QString> normalDisableActions;

private:
    CanvasMenuScene *q;
};

}
#endif   // CANVASMENUSCENE_P_H
