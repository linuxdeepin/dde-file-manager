// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "config.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>

#include <dfm-framework/dpf.h>

static void initLog()
{
    dpfLogManager->registerConsoleAppender();
    dpfLogManager->registerFileAppender();
}

static bool pluginsLoad()
{
    dpfCheckTimeBegin();

    dpfCheckTimeEnd();
    return true;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    a.setOrganizationName(DFM_BUILD_PLUGIN_DIR);

    DPF_NAMESPACE::backtrace::installStackTraceHandler();
    initLog();

    if (!pluginsLoad()) {
        qCritical() << "Load pugin failed!";
        abort();
    }

    int ret { a.exec() };
    DPF_NAMESPACE::LifeCycle::shutdownPlugins();
    return ret;
}
