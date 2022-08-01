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
#include "canvasinterface_p.h"
#include "fileinfomodelshell.h"

#include <dpf.h>

#include <QFileInfo>

DDP_ORGANIZER_USE_NAMESPACE

#define CanvasManagerPush(topic) \
        dpfSlotChannel->push("ddplugin_canvas", QT_STRINGIFY2(topic))


#define CanvasManagerPush2(topic, args...) \
        dpfSlotChannel->push("ddplugin_canvas", QT_STRINGIFY2(topic), ##args)

CanvasInterfacePrivate::CanvasInterfacePrivate(CanvasInterface *qq) : q(qq)
{

}

CanvasInterfacePrivate::~CanvasInterfacePrivate()
{
}

CanvasInterface::CanvasInterface(QObject *parent)
    : QObject(parent)
    , d(new CanvasInterfacePrivate(this))
{

}

CanvasInterface::~CanvasInterface()
{
    delete d;
    d = nullptr;
}

bool CanvasInterface::initialize()
{
    d->fileInfoModel = new FileInfoModelShell(this);
    d->fileInfoModel->initialize();

    d->canvaModel = new CanvasModelShell(this);
    d->canvaModel->initialize();

    d->canvasView = new CanvasViewShell(this);
    d->canvasView->initialize();

    d->canvasGrid = new CanvasGridShell(this);
    d->canvasGrid->initialize();

    return true;
}

int CanvasInterface::iconLevel()
{
    return CanvasManagerPush(slot_CanvasManager_IconLevel).toInt();
}

void CanvasInterface::setIconLevel(int lv)
{
    CanvasManagerPush2(slot_CanvasManager_SetIconLevel, lv);
}

FileInfoModelShell *CanvasInterface::fileInfoModel()
{
    return d->fileInfoModel;
}

CanvasModelShell *CanvasInterface::canvasModel()
{
    return d->canvaModel;
}

CanvasViewShell *CanvasInterface::canvasView()
{
    return d->canvasView;
}

CanvasGridShell *CanvasInterface::canvasGrid()
{
    return d->canvasGrid;
}


