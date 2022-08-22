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
#ifndef CANVASINTERFACE_P_H
#define CANVASINTERFACE_P_H

#include "canvasinterface.h"
#include "fileinfomodelshell.h"
#include "canvasmodelshell.h"
#include "canvasviewshell.h"
#include "canvasgridshell.h"
#include "canvasmanagershell.h"

namespace ddplugin_organizer {

class CanvasInterfacePrivate
{
public:
    explicit CanvasInterfacePrivate(CanvasInterface *qq);
    ~CanvasInterfacePrivate();
public:
   FileInfoModelShell *fileInfoModel = nullptr;
   CanvasModelShell *canvaModel = nullptr;
   CanvasViewShell *canvasView = nullptr;
   CanvasGridShell *canvasGrid = nullptr;
   CanvasManagerShell *canvasManager = nullptr;
private:
    CanvasInterface *q;
};

}

#endif // CANVASINTERFACE_P_H
