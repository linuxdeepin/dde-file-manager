// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CANVASMENUSCENE_P_H
#define CANVASMENUSCENE_P_H
#include "canvasmenuscene.h"
#include "canvasmenu_defines.h"

#include "plugins/common/dfmplugin-menu/menuscene/action_defines.h"

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
    bool checkOrganizerPlugin();

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
