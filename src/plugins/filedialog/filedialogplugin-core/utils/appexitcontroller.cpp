/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#include "appexitcontroller.h"

#include <QDebug>

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
        qInfo() << "Ready to exit: " << (totalSeconds - curSeconds);
        return;
    }
    qWarning() << "App exit!";
    if (!confirmFunc)
        exit(0);
    if (confirmFunc && confirmFunc())
        exit(0);
    qWarning() << "App exit failed";
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
        qWarning() << "Timer is active, cannot start again";
        return;
    }

    qInfo() << "Ready to exit: " << seconds;
    curSeconds = 0;
    totalSeconds = seconds;
    confirmFunc = confirm;
    exitTimer->start(1000);
}

void AppExitController::dismiss()
{
    if (!exitTimer->isActive()) {
        qWarning() << "Timer is active, canot stop";
        return;
    }

    qInfo() << "Dismiss exit";
    curSeconds = 0;
    totalSeconds = 0;
    exitTimer->stop();
}
