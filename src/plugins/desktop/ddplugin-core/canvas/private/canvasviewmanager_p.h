/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
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
#ifndef CANVASVIEWMANAGER_P_H
#define CANVASVIEWMANAGER_P_H

#include "defaultcanvasview_p.h"
#include "canvasviewmanager.h"

#include "dfm-base/dfm_base_global.h"

DFMBASE_USE_NAMESPACE
DSB_D_BEGIN_NAMESPACE

typedef QSharedPointer<DefaultCanvasView> canvasViewPointer;
class CanvasViewManagerPrivate : public QObject
{
    Q_OBJECT
    friend class CanvasViewManager;

public:
    explicit CanvasViewManagerPrivate(CanvasViewManager *qq)
        : QObject(qq), q(qq)
    {
    }

    ~CanvasViewManagerPrivate()
    {
        canvasViewMap.clear();
        screenMap.clear();
    }

    inline QRect relativeRect(const QRect &avRect, const QRect &geometry)
    {
        QPoint relativePos = avRect.topLeft() - geometry.topLeft();
        return QRect(relativePos, avRect.size());
    }

public:
    bool isDone { false };
    dfmbase::AbstractCanvasModel *canvasModel = { nullptr };
    dfmbase::AbstractCanvasSelectionModel *canvasSelectModel { nullptr };
    QItemSelectionModel *selectModel = nullptr;
    ScreenService *screenScevice { nullptr };
    BackgroundService *backgroundService { nullptr };
    QMap<QString, canvasViewPointer> canvasViewMap;
    QMap<QString, ScreenPointer> screenMap;
    CanvasViewManager *const q { nullptr };
};

DSB_D_END_NAMESPACE
#endif   // CANVASVIEWMANAGER_P_H
