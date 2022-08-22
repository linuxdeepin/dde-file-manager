/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangchunlin<wangchunlin@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
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
#include "canvasmanagershell.h"

#include <dfm-framework/dpf.h>

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
}

bool CanvasManagerShell::initialize()
{
    CanvasManagerSubscribe(signal_CanvasManager_IconSizeChanged, &CanvasManagerShell::iconSizeChanged);
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
