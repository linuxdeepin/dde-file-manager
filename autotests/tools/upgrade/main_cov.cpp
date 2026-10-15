// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Custom main for ut-dfm-upgrade-cov: redirects HOME / XDG_* locations to a
// temporary directory BEFORE the QApplication exists, so every upgrade unit
// (QStandardPaths / QDir::homePath / dconfig / sqlite) reads and writes in an
// isolated sandbox instead of the developer's real $HOME.

#include <gtest/gtest.h>

#include <QApplication>
#include <QDir>
#include <QTemporaryDir>

#include "dfm_test_main.h"

int main(int argc, char **argv)
{
    static QTemporaryDir sandbox;
    if (sandbox.isValid()) {
        const QString home = sandbox.path();
        qputenv("HOME", home.toUtf8());
        qputenv("XDG_CONFIG_HOME", QString(home + "/.config").toUtf8());
        qputenv("XDG_CACHE_HOME", QString(home + "/.cache").toUtf8());
        qputenv("XDG_DATA_HOME", QString(home + "/.local/share").toUtf8());
        QDir().mkpath(home + "/.config");
        QDir().mkpath(home + "/.cache");
        QDir().mkpath(home + "/.local/share");
    }

    QApplication app(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    int result = RUN_ALL_TESTS();
    DFM_SETUP_ASAN_REPORT(dfm_upgrade_cov);
    return result;
}
