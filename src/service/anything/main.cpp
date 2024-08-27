// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "anythingserver.h"

#include <QCoreApplication>
#include <QDebug>

SERVICEANYTHING_USE_NAMESPACE

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    AnythingPlugin plugin;

    plugin.initialize();
    bool started = plugin.start();
    if (!started) {
        fmWarning() << "Start deepin anything service failed!";
        return a.exec();
    }

    auto ret = a.exec();
    plugin.stop();

    return ret;
}
