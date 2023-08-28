// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "canvasinterface_p.h"

#include <dfm-framework/dpf.h>

#include <QFileInfo>

using namespace ddplugin_organizer;

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


