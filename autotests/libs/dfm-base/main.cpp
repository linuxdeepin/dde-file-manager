// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QApplication>
#include <QDir>
#include <QTemporaryDir>
#include <cstdlib>

#include "stubext.h"
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/settings.h>

#include "dfm_test_main.h"

int main(int argc, char** argv) {
    // Redirect ALL config files (QSettings, DConf/dconf, DConfig) to a temp
    // directory so that unit tests can never modify the user's real config
    // files (e.g. ~/.config/deepin/dde-file-manager/*.json, dconf DB, etc.).
    static QTemporaryDir configHome;
    if (configHome.isValid()) {
        qputenv("XDG_CONFIG_HOME", configHome.path().toUtf8());
    }

    QApplication app(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);

    // Globally stub all config-write paths to prevent ANY modification of the
    // user's real environment:
    //
    // 1. DConfigManager::setValue — DConfigManager's constructor auto-registers
    //    org.deepin.dde.file-manager and .view schemas via DConfig::create(),
    //    which connects to the session dconf D-Bus service.  setValue forwards
    //    to DConfig::setValue which writes to the real dconf DB via D-Bus,
    //    regardless of XDG_CONFIG_HOME.
    //
    // 2. Settings::sync — Settings objects use autoSync=true, so every
    //    setValue triggers a sync that writes to ~/.config/deepin/*.json.
    //    XDG_CONFIG_HOME should redirect this, but stubbing sync is a second
    //    layer of defense.
    //
    // Both are non-virtual, so we use ADDR (plain PMF) — StubExt::set_lamda
    // patches the function's machine code directly.  The stubs live for the
    // entire test process.
    static stub_ext::StubExt g_stub;
    g_stub.set_lamda(ADDR(dfmbase::DConfigManager, setValue),
                     [](dfmbase::DConfigManager *, const QString &, const QString &, const QVariant &) {
                         __DBG_STUB_INVOKE__
                     });
    g_stub.set_lamda(ADDR(dfmbase::Settings, sync),
                     [](dfmbase::Settings *) -> bool {
                         __DBG_STUB_INVOKE__
                         return true;
                     });

    int result = RUN_ALL_TESTS();
    DFM_SETUP_ASAN_REPORT(dfm_base);
    return result;
}
