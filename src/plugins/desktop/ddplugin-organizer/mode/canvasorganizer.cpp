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
#include "canvasorganizer.h"
#include "normalizedmode.h"
#include "custommode.h"
#include "interface/canvasmodelshell.h"

DDP_ORGANIZER_USE_NAMESPACE

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

void CanvasOrganizer::setCanvasShell(CanvasModelShell *sh)
{
    if (sh == shell)
        return;

    if (shell)
        disconnect(shell, nullptr, this, nullptr);

    shell = sh;
    if (!shell)
        return;

    connect(shell, &CanvasModelShell::filterDataRested, this, &CanvasOrganizer::filterDataRested, Qt::DirectConnection);
    connect(shell, &CanvasModelShell::filterDataInserted, this, &CanvasOrganizer::filterDataInserted, Qt::DirectConnection);
    connect(shell, &CanvasModelShell::filterDataRenamed, this, &CanvasOrganizer::filterDataRenamed, Qt::DirectConnection);

}

void CanvasOrganizer::setSurface(QWidget *w)
{
    surface = w;
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


