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
#include <QPoint>
#include <QMetaMethod>

Q_DECLARE_METATYPE(QList<QUrl> *)

using namespace ddplugin_organizer;

#define CanvasModelPush(topic, args...) \
        dpfSlotChannel->push("ddplugin_canvas", QT_STRINGIFY2(topic), ##args)

#define CanvasModelFollow(topic, args...) \
        dpfHookSequence->follow("ddplugin_canvas", QT_STRINGIFY2(topic), this, ##args)

#define CanvasModelUnfollow(topic, args...) \
        dpfHookSequence->unfollow("ddplugin_canvas", QT_STRINGIFY2(topic), this, ##args)

#define CheckFilterConnected(sig) \
        if (!isSignalConnected(QMetaMethod::fromSignal(&sig))) {\
            qWarning() << "filter signal was not connected to any object" << #sig;\
            return false; \
        }

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

void CanvasModelShell::refresh(int ms)
{
    CanvasModelPush(slot_CanvasModel_Refresh, false, ms);
}

bool CanvasModelShell::fetch(const QUrl &url)
{
    return CanvasModelPush(slot_CanvasModel_Fetch, url).toBool();
}

bool CanvasModelShell::take(const QUrl &url)
{
    return CanvasModelPush(slot_CanvasModel_Take, url).toBool();
}

bool CanvasModelShell::eventDataRested(QList<QUrl> *urls, void *extData)
{
    Q_UNUSED(extData);
    CheckFilterConnected(CanvasModelShell::filterDataRested)
    return filterDataRested(urls);
}

bool CanvasModelShell::eventDataInserted(const QUrl &url, void *extData)
{
    Q_UNUSED(extData);
    CheckFilterConnected(CanvasModelShell::filterDataInserted)
    return filterDataInserted(url);
}

bool CanvasModelShell::eventDataRenamed(const QUrl &oldUrl, const QUrl &newUrl, void *extData)
{
    Q_UNUSED(extData);
    CheckFilterConnected(CanvasModelShell::filterDataRenamed)
    return filterDataRenamed(oldUrl, newUrl);
}


