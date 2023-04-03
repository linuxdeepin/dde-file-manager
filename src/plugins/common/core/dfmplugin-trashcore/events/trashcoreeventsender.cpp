// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "trashcoreeventsender.h"
#include "utils/trashcorehelper.h"

#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/base/standardpaths.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/localfilewatcher.h>
#include <dfm-base/interfaces/abstractfilewatcher.h>

#include <dfm-framework/dpf.h>

#include <QDebug>
#include <QUrl>

#include <functional>

using namespace dfmplugin_trashcore;
DFMBASE_USE_NAMESPACE

TrashCoreEventSender::TrashCoreEventSender(QObject *parent)
    : QObject(parent)
{
    isEmpty = FileUtils::trashIsEmpty();
    initTrashWatcher();
}

void TrashCoreEventSender::initTrashWatcher()
{
    trashFileWatcher.reset(new LocalFileWatcher(FileUtils::trashRootUrl(), this));

    connect(trashFileWatcher.data(), &AbstractFileWatcher::subfileCreated, this, &TrashCoreEventSender::sendTrashStateChangedAdd);
    connect(trashFileWatcher.data(), &AbstractFileWatcher::fileDeleted, this, &TrashCoreEventSender::sendTrashStateChangedDel);
    trashFileWatcher->startWatcher();
}

TrashCoreEventSender *TrashCoreEventSender::instance()
{
    static TrashCoreEventSender sender;
    return &sender;
}

void TrashCoreEventSender::sendTrashStateChangedDel()
{
    bool empty = FileUtils::trashIsEmpty();
    if (empty == isEmpty)
        return;

    isEmpty = !isEmpty;

    if (!isEmpty)
        return;

    dpfSignalDispatcher->publish("dfmplugin_trashcore", "signal_TrashCore_TrashStateChanged");
}

void TrashCoreEventSender::sendTrashStateChangedAdd()
{
    if (!isEmpty)
        return;

    isEmpty = false;

    dpfSignalDispatcher->publish("dfmplugin_trashcore", "signal_TrashCore_TrashStateChanged");
}
