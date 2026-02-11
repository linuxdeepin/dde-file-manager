// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "canvasselectionhook.h"

#include <dfm-framework/dpf.h>

using namespace ddplugin_canvas;

CanvasSelectionHook::CanvasSelectionHook(QObject *parent)
    : QObject(parent), SelectionHookInterface()
{

}

void CanvasSelectionHook::clear() const
{
    dpfSignalDispatcher->publish(QT_STRINGIFY(DDP_CANVAS_NAMESPACE), "signal_CanvasSelectionModel_Clear");
}
