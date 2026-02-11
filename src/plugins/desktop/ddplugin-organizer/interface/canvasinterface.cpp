// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "canvasinterface_p.h"

#include <dfm-framework/dpf.h>

#include <QFileInfo>

using namespace ddplugin_organizer;

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

    d->canvasManager = new CanvasManagerShell(this);
    d->canvasManager->initialize();

    d->canvasSelectionShell = new CanvasSelectionShell(this);
    d->canvasSelectionShell->initialize();

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

CanvasManagerShell *CanvasInterface::canvasManager()
{
    return d->canvasManager;
}

CanvasSelectionShell *CanvasInterface::canvasSelectionShell()
{
    return d->canvasSelectionShell;
}


