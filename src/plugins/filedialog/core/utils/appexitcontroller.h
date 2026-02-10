// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
