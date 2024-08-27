// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ANYTHING_H
#define ANYTHING_H

#include "service_anything_global.h"

#include <dfm-framework/dpf.h>
#include <QProcess>
#include <QSemaphore>

SERVICEANYTHING_BEGIN_NAMESPACE

class AnythingPlugin : public QObject
{
    Q_OBJECT
public:
    void initialize();
    bool start();
    void stop();

private:
    bool startAnythingByLib();
    void stopAnythingByLib();

private:
    QLibrary *backendLib;
    bool stopped;
};

class AnythingMonitorThread : public QThread
{
    Q_OBJECT
public:
    explicit AnythingMonitorThread(bool *stopped) : QThread(nullptr)
    {
        this->stopped = stopped;
    }

    void run() override;

    bool waitStartResult()
    {
        start();
        sem.acquire();
        return started;
    }

private:
    QSemaphore sem;
    bool started;
    bool *stopped;
};

SERVICEANYTHING_END_NAMESPACE
#endif   // ANYTHING_H
