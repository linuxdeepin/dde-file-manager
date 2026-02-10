// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "appexitcontroller.h"

#include <QDebug>
#include <QApplication>

using namespace filedialog_core;

AppExitController::AppExitController(QObject *parent)
    : QObject(parent)
{
    connect(exitTimer.data(), &QTimer::timeout, this, &AppExitController::onExit);
}

void AppExitController::onExit()
{
    ++curSeconds;
    if (curSeconds < totalSeconds) {
        fmDebug() << "File Dialog: Ready to exit: " << (totalSeconds - curSeconds);
        return;
    }
    fmWarning() << "File Dialog: App exit!";
    if (!confirmFunc)
        qApp->exit(0);
    if (confirmFunc())
        qApp->exit(0);
    fmWarning() << "File Dialog: App exit failed!";
}

AppExitController &AppExitController::instance()
{
    static AppExitController ins;
    return ins;
}

void AppExitController::readyToExit(int seconds, ExitConfirmFunc confirm)
{
    Q_ASSERT(seconds >= 0);
    Q_ASSERT(seconds <= 600);

    if (exitTimer->isActive()) {
        fmWarning() << "File Dialog: Timer is active, cannot start again.";
        return;
    }

    fmInfo() << "File Dialog: Ready to exit: " << seconds;
    curSeconds = 0;
    totalSeconds = seconds;
    confirmFunc = confirm;
    exitTimer->start(1000);
}

void AppExitController::dismiss()
{
    if (!exitTimer->isActive()) {
        fmWarning() << "File Dialog: Timer is active, canot stop.";
        return;
    }

    fmInfo() << "File Dialog: Dismiss exit.";
    curSeconds = 0;
    totalSeconds = 0;
    exitTimer->stop();
}
