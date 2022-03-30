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

#include <services/common/dfm_common_service_global.h>
#include <services/common/menu/menuservice.h>

#include <QMap>

DSC_USE_NAMESPACE
DDP_CANVAS_BEGIN_NAMESPACE

class CanvasView;
class CanvasMenuScenePrivate
{
public:
    explicit CanvasMenuScenePrivate(CanvasMenuScene *qq);

    QString currentDir;
    QStringList selectFiles;
    QString focusFile;
    bool onDesktop { false };
    bool isEmptyArea { true };
    QMap<QAction *, int> iconSizeAction;

    QMap<QString, QString> predicateName;
    QMap<QString, QAction *> predicateAction;

    CanvasView *view { nullptr };
    MenuService *menuServer = nullptr;
private:
    CanvasMenuScene *q;
};

DDP_CANVAS_END_NAMESPACE
#endif   // CANVASMENUSCENE_P_H
