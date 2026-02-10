// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "canvasdbusinterface.h"
#include "private/canvasmanager_p.h"

using namespace ddplugin_canvas;
CanvasDBusInterface::CanvasDBusInterface(CanvasManager *parent)
    : QObject(parent)
    , QDBusContext()
    , manager(parent)
{

}

void CanvasDBusInterface::EnableUIDebug(bool enable)
{
    for (auto view : manager->views()) {
        view->showGrid(enable);
        view->update();
    }
}

void CanvasDBusInterface::Refresh(bool silent)
{
    manager->refresh(silent);
}
