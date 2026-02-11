// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "canvasmanagershell.h"

#include <dfm-framework/dpf.h>

#include <QItemSelectionModel>

using namespace ddplugin_organizer;

#define CanvasManagerSubscribe(topic, func) \
    dpfSignalDispatcher->subscribe("ddplugin_canvas", QT_STRINGIFY2(topic), this, func);

#define CanvasManagerUnsubscribe(topic, func) \
    dpfSignalDispatcher->unsubscribe("ddplugin_canvas", QT_STRINGIFY2(topic), this, func);

#define CanvasManagerPush(topic) \
    dpfSlotChannel->push("ddplugin_canvas", QT_STRINGIFY2(topic))

#define CanvasManagerPush2(topic, args...) \
    dpfSlotChannel->push("ddplugin_canvas", QT_STRINGIFY2(topic), ##args)

CanvasManagerShell::CanvasManagerShell(QObject *parent)
    : QObject(parent)
{
}

CanvasManagerShell::~CanvasManagerShell()
{
    CanvasManagerUnsubscribe(signal_CanvasManager_IconSizeChanged, &CanvasManagerShell::iconSizeChanged);
    CanvasManagerUnsubscribe(signal_CanvasManager_FontChanged, &CanvasManagerShell::fontChanged);
    CanvasManagerUnsubscribe(signal_CanvasView_RequestRefresh, &CanvasManagerShell::requestRefresh);
}

bool CanvasManagerShell::initialize()
{
    CanvasManagerSubscribe(signal_CanvasManager_IconSizeChanged, &CanvasManagerShell::iconSizeChanged);
    CanvasManagerSubscribe(signal_CanvasManager_FontChanged, &CanvasManagerShell::fontChanged);
    CanvasManagerSubscribe(signal_CanvasView_RequestRefresh, &CanvasManagerShell::requestRefresh);
    return true;
}

int CanvasManagerShell::iconLevel() const
{
    return CanvasManagerPush(slot_CanvasManager_IconLevel).toInt();
}

void CanvasManagerShell::setIconLevel(const int level)
{
    CanvasManagerPush2(slot_CanvasManager_SetIconLevel, level);
}
