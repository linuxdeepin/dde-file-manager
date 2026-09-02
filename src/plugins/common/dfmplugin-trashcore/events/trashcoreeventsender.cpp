// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "trashcoreeventsender.h"
#include "trashcorestartupprobe.h"

#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/base/standardpaths.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/trashutils.h>
#include <dfm-base/utils/networkutils.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/localfilewatcher.h>
#include <dfm-base/interfaces/abstractfilewatcher.h>

#include <dfm-framework/dpf.h>

#include <QDebug>
#include <QUrl>
#include <QDir>
#include <QtConcurrent>
#include <QFutureWatcher>

using namespace dfmplugin_trashcore;
DFMBASE_USE_NAMESPACE

TrashCoreEventSender::TrashCoreEventSender(QObject *parent)
    : QObject(parent)
{
    TrashUtils::setTrashEmptyState(TrashUtils::TrashEmptyState::kUnknown);

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
    if (!trashWatchers.isEmpty())
        return;

    const auto &dirs = TrashUtils::localTrashDirs();
    for (const QString &dir : dirs) {
        if (!QDir(dir).exists())
            continue;
        QUrl localUrl = QUrl::fromLocalFile(dir);
        auto watcher = QSharedPointer<AbstractFileWatcher>(new LocalFileWatcher(localUrl, this));
        connect(watcher.data(), &AbstractFileWatcher::subfileCreated, this, &TrashCoreEventSender::sendTrashStateChangedAdd);
        connect(watcher.data(), &AbstractFileWatcher::fileDeleted, this, &TrashCoreEventSender::sendTrashStateChangedDel);
        trashWatchers.append(watcher);
    }

    if (trashWatchers.isEmpty())
        fmWarning() << "TrashCore: No local trash directories found for watching";
}

bool TrashCoreEventSender::checkAndStartWatcher()
{
    if (cifsCheckInProgress)
        return false;

    cifsCheckInProgress = true;
    auto *watcher = new QFutureWatcher<bool>(this);
    connect(watcher, &QFutureWatcher<bool>::finished, this, [this, watcher]() {
        cifsCheckInProgress = false;
        bool cifsBusy = watcher->result();
        watcher->deleteLater();

        if (cifsBusy) {
            timer.start();
            return;
        }

        bool allStarted = true;
        for (auto &w : trashWatchers) {
            if (!w->startWatcher()) {
                allStarted = false;
            }
        }

        if (!allStarted) {
            timer.start();
            return;
        }

        watcherInitialized = true;
        initTrashState();
    });
    watcher->setFuture(QtConcurrent::run([]() {
        return NetworkUtils::instance()->checkAllCIFSBusy();
    }));

    return false;
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
    const bool actuallyEmpty = TrashUtils::trashIsEmpty();
    trashState = actuallyEmpty ? TrashState::Empty : TrashState::NotEmpty;
    TrashUtils::setTrashEmptyState(actuallyEmpty ? TrashUtils::TrashEmptyState::kEmpty
                                                : TrashUtils::TrashEmptyState::kNotEmpty);

    // Startup defaults to the non-empty icon, so only an empty result needs
    // an immediate correction signal.
    if (trashState == TrashState::Empty)
        dpfSignalDispatcher->publish("dfmplugin_trashcore", "signal_TrashCore_TrashStateChanged");
}

void TrashCoreEventSender::sendTrashStateChangedDel()
{
    bool actuallyEmpty = TrashUtils::trashIsEmpty();
    TrashState newState = actuallyEmpty ? TrashState::Empty : TrashState::NotEmpty;
    TrashUtils::setTrashEmptyState(actuallyEmpty ? TrashUtils::TrashEmptyState::kEmpty
                                                : TrashUtils::TrashEmptyState::kNotEmpty);

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
    TrashUtils::setTrashEmptyState(TrashUtils::TrashEmptyState::kNotEmpty);
    if (trashState == TrashState::Unknown || trashState == TrashState::Empty) {
        trashState = TrashState::NotEmpty;
        qInfo() << "TrashCore: Trash became non-empty, sending state changed signal";
        dpfSignalDispatcher->publish("dfmplugin_trashcore", "signal_TrashCore_TrashStateChanged");
    }
    // If trash was already not empty, no state change occurred, no signal needed
}
