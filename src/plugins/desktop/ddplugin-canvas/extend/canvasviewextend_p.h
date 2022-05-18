/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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
#ifndef CANVASVIEWEXTEND_P_H
#define CANVASVIEWEXTEND_P_H

#include "ddplugin_canvas_global.h"
#include "canvasviewextend.h"
#include "canvaseventprovider.h"

#include <QObject>

DDP_CANVAS_BEGIN_NAMESPACE

static constexpr char kFilterCanvasViewContextMenu[] = "CanvasView_Filter_contextMenu";
static constexpr char kFilterCanvasViewDropData[] = "CanvasView_Filter_dropData";
static constexpr char kFilterCanvasViewKeyPress[] = "CanvasView_Filter_keyPress";
static constexpr char kFilterCanvasViewMousePress[] = "CanvasView_Filter_mousePress";
static constexpr char kFilterCanvasViewMouseRelease[] = "CanvasView_Filter_mouseRelease";
static constexpr char kFilterCanvasViewMouseDoubleClick[] = "CanvasView_Filter_mouseDoubleClick";
static constexpr char kFilterCanvasViewWheel[] = "CanvasView_Filter_wheel";
static constexpr char kFilterCanvasViewStartDrag[] = "CanvasView_Filter_startDrag";
static constexpr char kFilterCanvasViewDragEnter[] = "CanvasView_Filter_dragEnter";
static constexpr char kFilterCanvasViewDragMove[] = "CanvasView_Filter_dragMove";
static constexpr char kFilterCanvasViewDragLeave[] = "CanvasView_Filter_dragLeave";
static constexpr char kFilterCanvasViewKeyboardSearch[] = "CanvasView_Filter_keyboardSearch";
static constexpr char kFilterCanvasViewDrawFile[] = "CanvasView_Filter_drawFile";

class CanvasViewExtendPrivate : public QObject, public CanvasEventProvider
{
public:
    explicit CanvasViewExtendPrivate(CanvasViewExtend *qq);
    ~CanvasViewExtendPrivate() override;
protected:
    virtual void registerEvent() override;
private:
    CanvasViewExtend *q;
};

DDP_CANVAS_END_NAMESPACE

#endif // CANVASVIEWEXTEND_P_H
