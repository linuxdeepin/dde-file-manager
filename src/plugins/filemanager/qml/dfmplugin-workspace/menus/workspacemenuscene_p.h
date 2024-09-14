// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WORKSPACEMENUSCENE_P_H
#define WORKSPACEMENUSCENE_P_H

#include "workspacemenuscene.h"
#include "workspacemenu_defines.h"

#include "plugins/common/core/dfmplugin-menu/menuscene/action_defines.h"

#include <dfm-base/dfm_menu_defines.h>
#include <dfm-base/interfaces/private/abstractmenuscene_p.h>

namespace dfmplugin_workspace {

class FileView;
class WorkspaceMenuScenePrivate : public DFMBASE_NAMESPACE::AbstractMenuScenePrivate
{
    Q_OBJECT
public:
    explicit WorkspaceMenuScenePrivate(WorkspaceMenuScene *qq);

public:
    FileView *view = nullptr;

private:
    WorkspaceMenuScene *q;
};

}

#endif   // WORKSPACEMENUSCENE_P_H
