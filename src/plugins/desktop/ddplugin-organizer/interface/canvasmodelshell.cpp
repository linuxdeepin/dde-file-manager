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
#include "canvasmodelshell.h"

#include "interface/canvasinterface.h"

#include "dfm-framework/dpf.h"

#include <QUrl>
#include <QMimeData>

Q_DECLARE_METATYPE(QList<QUrl> *)

DDP_ORGANIZER_USE_NAMESPACE

#define CanvasModelFollow(topic, args...) \
        dpfHookSequence->follow("ddplugin_canvas", QT_STRINGIFY2(topic), this, ##args)

#define CanvasModelUnfollow(topic, args...) \
        dpfHookSequence->unfollow("ddplugin_canvas", QT_STRINGIFY2(topic), this, ##args)

CanvasModelShell::CanvasModelShell(QObject *parent)
    : QObject(parent)
{

}

CanvasModelShell::~CanvasModelShell()
{
    CanvasModelUnfollow(hook_CanvasModel_DataRested, &CanvasModelShell::eventDataRested);
    CanvasModelUnfollow(hook_CanvasModel_DataInserted, &CanvasModelShell::eventDataInserted);
    CanvasModelUnfollow(hook_CanvasModel_DataRenamed, &CanvasModelShell::eventDataRenamed);
}

bool CanvasModelShell::initialize()
{
    CanvasModelFollow(hook_CanvasModel_DataRested, &CanvasModelShell::eventDataRested);
    CanvasModelFollow(hook_CanvasModel_DataInserted, &CanvasModelShell::eventDataInserted);
    CanvasModelFollow(hook_CanvasModel_DataRenamed, &CanvasModelShell::eventDataRenamed);
    return true;
}

bool CanvasModelShell::eventDataRested(QList<QUrl> *urls, void *extData)
{
    Q_UNUSED(extData);
    return filterDataRested(urls);
}

bool CanvasModelShell::eventDataInserted(const QUrl &url, void *extData)
{
    Q_UNUSED(extData);
    return filterDataInserted(url);
}

bool CanvasModelShell::eventDataRenamed(const QUrl &oldUrl, const QUrl &newUrl, void *extData)
{
    Q_UNUSED(extData);
    return filterDataRenamed(oldUrl, newUrl);
}


