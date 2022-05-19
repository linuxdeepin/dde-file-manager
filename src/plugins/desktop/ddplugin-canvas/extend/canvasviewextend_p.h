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

inline constexpr char kFilterCanvasViewContextMenu[] = "CanvasView_Filter_contextMenu";
inline constexpr char kFilterCanvasViewDropData[] = "CanvasView_Filter_dropData";
inline constexpr char kFilterCanvasViewKeyPress[] = "CanvasView_Filter_keyPress";
inline constexpr char kFilterCanvasViewMousePress[] = "CanvasView_Filter_mousePress";
inline constexpr char kFilterCanvasViewMouseRelease[] = "CanvasView_Filter_mouseRelease";
inline constexpr char kFilterCanvasViewMouseDoubleClick[] = "CanvasView_Filter_mouseDoubleClick";
inline constexpr char kFilterCanvasViewWheel[] = "CanvasView_Filter_wheel";
inline constexpr char kFilterCanvasViewStartDrag[] = "CanvasView_Filter_startDrag";
inline constexpr char kFilterCanvasViewDragEnter[] = "CanvasView_Filter_dragEnter";
inline constexpr char kFilterCanvasViewDragMove[] = "CanvasView_Filter_dragMove";
inline constexpr char kFilterCanvasViewDragLeave[] = "CanvasView_Filter_dragLeave";
inline constexpr char kFilterCanvasViewKeyboardSearch[] = "CanvasView_Filter_keyboardSearch";
inline constexpr char kFilterCanvasViewDrawFile[] = "CanvasView_Filter_drawFile";

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

#endif   // CANVASVIEWEXTEND_P_H
