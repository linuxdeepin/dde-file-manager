// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
