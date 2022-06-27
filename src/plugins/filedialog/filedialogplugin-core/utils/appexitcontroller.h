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
#ifndef APPEXITCONTROLLER_H
#define APPEXITCONTROLLER_H

#include "filedialogplugin_core_global.h"

#include <QObject>
#include <QTimer>

#include <functional>

namespace filedialog_core {

class AppExitController : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(AppExitController)

public:
    using ExitConfirmFunc = std::function<bool()>;

public:
    static AppExitController &instance();

    void readyToExit(int seconds, ExitConfirmFunc confirm = nullptr);
    void dismiss();

private:
    explicit AppExitController(QObject *parent = nullptr);

private slots:
    void onExit();

private:
    QScopedPointer<QTimer> exitTimer { new QTimer };
    ExitConfirmFunc confirmFunc {};
    int curSeconds { 0 };
    int totalSeconds { 0 };
};

}

#endif   // APPEXITCONTROLLER_H
