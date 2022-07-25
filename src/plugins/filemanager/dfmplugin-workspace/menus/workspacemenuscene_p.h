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

#include "plugins/common/dfmplugin-menu/menuscene/action_defines.h"

#include "dfm-base/dfm_menu_defines.h"

#include <QGSettings>

namespace dfmplugin_workspace {

class FileView;
class WorkspaceMenuScenePrivate : public DFMBASE_NAMESPACE::AbstractMenuScenePrivate
{
    Q_OBJECT
public:
    explicit WorkspaceMenuScenePrivate(WorkspaceMenuScene *qq);

    void sortMenuAction(QMenu *menu, const QStringList &sortRule);

    inline QStringList emptyMenuActionRule()
    {
        static QStringList actionRule {
            dfmplugin_menu::ActionID::kNewFolder,
            dfmplugin_menu::ActionID::kNewDoc,
            dfmplugin_menu::ActionID::kSeparator,
            ActionID::kDisplayAs,
            ActionID::kSortBy,
            dfmplugin_menu::ActionID::kOpenAsAdmin,
            dfmplugin_menu::ActionID::kOpenInTerminal,
            ActionID::kRefresh,
            dfmplugin_menu::ActionID::kSeparator,
            dfmplugin_menu::ActionID::kPaste,
            dfmplugin_menu::ActionID::kSelectAll
        };

        return actionRule;
    }

    inline QStringList normalMenuActionRule()
    {
        static QStringList actionRule {
            dfmplugin_menu::ActionID::kOpen,
            dfmplugin_menu::ActionID::kOpenWith,
            dfmplugin_menu::ActionID::kSeparator,
            dfmplugin_menu::ActionID::kOpenInNewWindow,
            dfmplugin_menu::ActionID::kOpenInNewTab,
            dfmplugin_menu::ActionID::kOpenAsAdmin,
            dfmplugin_menu::ActionID::kSeparator,
            dfmplugin_menu::ActionID::kEmptyTrash,
            dfmplugin_menu::ActionID::kCut,
            dfmplugin_menu::ActionID::kCopy,
            dfmplugin_menu::ActionID::kRename,
            dfmplugin_menu::ActionID::kDelete,
            dfmplugin_menu::ActionID::kSeparator,
            dfmplugin_menu::ActionID::kCreateSymlink,
            dfmplugin_menu::ActionID::kSendToDesktop,
            dfmplugin_menu::ActionID::kSendTo,
            dfmplugin_menu::ActionID::kSetAsWallpaper,
            dfmplugin_menu::ActionID::kSeparator,
            dfmplugin_menu::ActionID::kOpenInTerminal
        };

        return actionRule;
    }

    inline bool isRefreshOn() const
    {
        // the gsetting control for refresh action
        if (QGSettings::isSchemaInstalled("com.deepin.dde.filemanager.contextmenu")) {
            const QGSettings menuSwitch("com.deepin.dde.filemanager.contextmenu",
                                        "/com/deepin/dde/filemanager/contextmenu/");
            if (menuSwitch.keys().contains("refresh")) {
                auto showRefreh = menuSwitch.get("refresh");
                if (showRefreh.isValid())
                    return showRefreh.toBool();
            }
        }
        return false;
    }

public:
    FileView *view = nullptr;
private:
    WorkspaceMenuScene *q;
};

}

#endif   // WORKSPACEMENUSCENE_P_H
