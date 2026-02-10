// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "canvasmodelshell.h"

#include "interface/canvasinterface.h"

#include <dfm-framework/dpf.h>

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

#define CheckFilterConnected(sig)                                               \
    if (!isSignalConnected(QMetaMethod::fromSignal(&sig))) {                    \
        fmWarning() << "filter signal was not connected to any object" << #sig; \
        return false;                                                           \
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

void CanvasModelShell::refresh(int ms, bool updateFile)
{
    CanvasModelPush(slot_CanvasModel_Refresh, false, ms, updateFile);
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
    CheckFilterConnected(CanvasModelShell::filterDataRested);
    return filterDataRested(urls);
}

bool CanvasModelShell::eventDataInserted(const QUrl &url, void *extData)
{
    Q_UNUSED(extData);
    CheckFilterConnected(CanvasModelShell::filterDataInserted);
    return filterDataInserted(url);
}

bool CanvasModelShell::eventDataRenamed(const QUrl &oldUrl, const QUrl &newUrl, void *extData)
{
    Q_UNUSED(extData);
    CheckFilterConnected(CanvasModelShell::filterDataRenamed);
    return filterDataRenamed(oldUrl, newUrl);
}
