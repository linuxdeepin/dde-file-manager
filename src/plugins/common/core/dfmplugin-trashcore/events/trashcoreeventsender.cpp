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
#include "trashcoreeventsender.h"
#include "utils/trashcorehelper.h"

#include "dfm-base/dfm_global_defines.h"
#include "dfm-base/base/standardpaths.h"
#include "dfm-base/utils/fileutils.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/file/local/localfilewatcher.h"
#include "dfm-base/interfaces/abstractfilewatcher.h"

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
