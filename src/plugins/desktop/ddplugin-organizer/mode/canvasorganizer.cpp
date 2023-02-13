// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "canvasorganizer.h"
#include "normalizedmode.h"
#include "custommode.h"
#include "interface/canvasmodelshell.h"
#include "interface/canvasviewshell.h"
#include "interface/canvasgridshell.h"
#include "interface/canvasmanagershell.h"

#include <QMimeData>

using namespace ddplugin_organizer;

CanvasOrganizer *OrganizerCreator::createOrganizer(OrganizerMode mode)
{
    CanvasOrganizer *ret = nullptr;
    switch (mode) {
    case OrganizerMode::kNormalized:
        ret = new NormalizedMode();
        break;
    case OrganizerMode::kCustom:
        ret = new CustomMode();
        break;
    default:
        break;
    }
    return ret;
}

CanvasOrganizer::CanvasOrganizer(QObject *parent) : QObject(parent)
{

}

CanvasOrganizer::~CanvasOrganizer()
{

}

void CanvasOrganizer::layout()
{

}

void CanvasOrganizer::detachLayout()
{

}

void CanvasOrganizer::setCanvasModelShell(CanvasModelShell *sh)
{
    if (sh == canvasModelShell)
        return;

    if (canvasModelShell)
        disconnect(canvasModelShell, nullptr, this, nullptr);

    canvasModelShell = sh;
    if (!canvasModelShell)
        return;

    // hook canvas model, must be DirectConnection
    connect(canvasModelShell, &CanvasModelShell::filterDataRested, this, &CanvasOrganizer::filterDataRested, Qt::DirectConnection);
    connect(canvasModelShell, &CanvasModelShell::filterDataInserted, this, &CanvasOrganizer::filterDataInserted, Qt::DirectConnection);
    connect(canvasModelShell, &CanvasModelShell::filterDataRenamed, this, &CanvasOrganizer::filterDataRenamed, Qt::DirectConnection);
}

void CanvasOrganizer::setCanvasViewShell(CanvasViewShell *sh)
{
    if (sh == canvasViewShell)
        return;

    if (canvasViewShell)
        disconnect(canvasViewShell, nullptr, this, nullptr);

    canvasViewShell = sh;
    if (!canvasViewShell)
        return;

    // hook canvas view, must be DirectConnection
    connect(canvasViewShell, &CanvasViewShell::filterDropData, this, &CanvasOrganizer::filterDropData, Qt::DirectConnection);
}

void CanvasOrganizer::setCanvasGridShell(CanvasGridShell *sh)
{
    if (sh == canvasGridShell)
        return;

    if (canvasGridShell)
        disconnect(canvasGridShell, nullptr, this, nullptr);

    canvasGridShell = sh;
    if (!canvasGridShell)
        return;

    // hook canvas grid, must be DirectConnection
}

void CanvasOrganizer::setCanvasManagerShell(CanvasManagerShell *sh)
{
    if (sh == canvasManagerShell)
        return;

    if (canvasManagerShell)
        disconnect(canvasManagerShell, nullptr, this, nullptr);

    canvasManagerShell = sh;
    if (!canvasManagerShell)
        return;
}

void CanvasOrganizer::setSurfaces(const QList<SurfacePointer> &surface)
{
    this->surfaces = surface;
}

void CanvasOrganizer::reset()
{

}

bool CanvasOrganizer::filterDataRested(QList<QUrl> *urls)
{
    return false;
}

bool CanvasOrganizer::filterDataInserted(const QUrl &url)
{
    return false;
}

bool CanvasOrganizer::filterDataRenamed(const QUrl &oldUrl, const QUrl &newUrl)
{
    return false;
}

bool CanvasOrganizer::filterDropData(int viewIndex, const QMimeData *mimeData, const QPoint &viewPoint)
{
    return false;
}

