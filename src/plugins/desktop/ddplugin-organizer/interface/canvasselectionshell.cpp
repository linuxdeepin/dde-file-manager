// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "canvasselectionshell.h"

#include <dfm-framework/dpf.h>

#include <QItemSelectionModel>

using namespace ddplugin_organizer;

#define CanvasPush(topic) \
        dpfSlotChannel->push("ddplugin_canvas", QT_STRINGIFY2(topic))

#define CanvasSubscribe(topic, func) \
        dpfSignalDispatcher->subscribe("ddplugin_canvas", QT_STRINGIFY2(topic), this, func);

#define CanvasUnsubscribe(topic, func) \
        dpfSignalDispatcher->unsubscribe("ddplugin_canvas", QT_STRINGIFY2(topic), this, func);

CanvasSelectionShell::CanvasSelectionShell(QObject *parent) : QObject(parent)
{

}

CanvasSelectionShell::~CanvasSelectionShell()
{
    CanvasUnsubscribe(signal_CanvasSelectionModel_Clear, &CanvasSelectionShell::eventClear);
}

bool CanvasSelectionShell::initialize()
{
    CanvasSubscribe(signal_CanvasSelectionModel_Clear, &CanvasSelectionShell::eventClear);
    return true;
}

QItemSelectionModel *CanvasSelectionShell::selectionModel()
{
    return CanvasPush(slot_CanvasManager_SelectionModel).value<QItemSelectionModel *>();
}

void CanvasSelectionShell::eventClear()
{
    emit requestClear();
}
