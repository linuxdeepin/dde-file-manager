// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "trashcoreeventsender.h"
#include "trashcorestartupprobe.h"

#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/base/standardpaths.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/networkutils.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/localfilewatcher.h>
#include <dfm-base/interfaces/abstractfilewatcher.h>

#include <dfm-framework/dpf.h>

#include <QDebug>
#include <QUrl>

using namespace dfmplugin_trashcore;
DFMBASE_USE_NAMESPACE

TrashCoreEventSender::TrashCoreEventSender(QObject *parent)
    : QObject(parent)
{
    FileUtils::setTrashEmptyState(FileUtils::TrashEmptyState::kUnknown);

    timer.setSingleShot(true);
    timer.setInterval(5000);
    connect(&timer, &QTimer::timeout, this, &TrashCoreEventSender::tryInitialize);

    startupProbe = new TrashCoreStartupProbe(this);
    connect(startupProbe, &TrashCoreStartupProbe::ready,
            this, &TrashCoreEventSender::tryInitialize);
    startupProbe->start();
}

void TrashCoreEventSender::initTrashWatcher()
{
    if (trashFileWatcher)
        return;

    trashFileWatcher.reset(new LocalFileWatcher(FileUtils::trashRootUrl(), this));

    connect(trashFileWatcher.data(), &AbstractFileWatcher::subfileCreated, this, &TrashCoreEventSender::sendTrashStateChangedAdd);
    connect(trashFileWatcher.data(), &AbstractFileWatcher::fileDeleted, this, &TrashCoreEventSender::sendTrashStateChangedDel);
}

bool TrashCoreEventSender::checkAndStartWatcher()
{
    if (NetworkUtils::instance()->checkAllCIFSBusy()) {
        timer.start();
        return false;
    }

    if (!trashFileWatcher->startWatcher()) {
        timer.start();
        return false;
    }

    return true;
}

TrashCoreEventSender *TrashCoreEventSender::instance()
{
    static TrashCoreEventSender sender;
    return &sender;
}

void TrashCoreEventSender::tryInitialize()
{
    if (watcherInitialized || !startupProbe || !startupProbe->isReady())
        return;

    initTrashWatcher();
    if (!checkAndStartWatcher())
        return;

    watcherInitialized = true;
    initTrashState();
}

void TrashCoreEventSender::initTrashState()
{
    const bool actuallyEmpty = FileUtils::trashIsEmpty();
    trashState = actuallyEmpty ? TrashState::Empty : TrashState::NotEmpty;
    FileUtils::setTrashEmptyState(actuallyEmpty ? FileUtils::TrashEmptyState::kEmpty
                                                : FileUtils::TrashEmptyState::kNotEmpty);

    // Startup defaults to the non-empty icon, so only an empty result needs
    // an immediate correction signal.
    if (trashState == TrashState::Empty)
        dpfSignalDispatcher->publish("dfmplugin_trashcore", "signal_TrashCore_TrashStateChanged");
}

void TrashCoreEventSender::sendTrashStateChangedDel()
{
    bool actuallyEmpty = FileUtils::trashIsEmpty();
    TrashState newState = actuallyEmpty ? TrashState::Empty : TrashState::NotEmpty;
    FileUtils::setTrashEmptyState(actuallyEmpty ? FileUtils::TrashEmptyState::kEmpty
                                                : FileUtils::TrashEmptyState::kNotEmpty);

    // Only send signal if state actually changed
    if (trashState == TrashState::Unknown || newState != trashState) {
        trashState = newState;

        // Only send signal when trash becomes empty (files deleted)
        if (trashState == TrashState::Empty) {
            qInfo() << "TrashCore: Trash became empty, sending state changed signal";
            dpfSignalDispatcher->publish("dfmplugin_trashcore", "signal_TrashCore_TrashStateChanged");
        }
    }
}

void TrashCoreEventSender::sendTrashStateChangedAdd()
{
    // If trash was empty and files are being added, it's now not empty
    FileUtils::setTrashEmptyState(FileUtils::TrashEmptyState::kNotEmpty);
    if (trashState == TrashState::Unknown || trashState == TrashState::Empty) {
        trashState = TrashState::NotEmpty;
        qInfo() << "TrashCore: Trash became non-empty, sending state changed signal";
        dpfSignalDispatcher->publish("dfmplugin_trashcore", "signal_TrashCore_TrashStateChanged");
    }
    // If trash was already not empty, no state change occurred, no signal needed
}
