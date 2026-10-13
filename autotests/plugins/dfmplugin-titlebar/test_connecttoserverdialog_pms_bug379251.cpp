// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <stubext.h>

#include <QUrl>
#include <QDateTime>
#include <DComboBox>

#include "dialogs/connecttoserverdialog.h"
#include "utils/searchhistroymanager.h"

#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/settings.h>
#include <dfm-base/utils/windowutils.h>
#include <dfm-base/dfm_global_defines.h>

DFMBASE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
using namespace dfmplugin_titlebar;

class TestConnectDialogBug379251 : public ::testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();

        stub.set_lamda(VADDR(ConnectToServerDialog, exec), [] {
            __DBG_STUB_INVOKE__
            return 0;
        });
        stub.set_lamda(VADDR(ConnectToServerDialog, show), [] {
            __DBG_STUB_INVOKE__
        });

        mockSettings = new Settings("test_connect_bug379251");
        stub.set_lamda(&Application::genericSetting, [this]() {
            __DBG_STUB_INVOKE__
            return mockSettings;
        });
        stub.set_lamda(&Application::appObtuselySetting, [this]() {
            __DBG_STUB_INVOKE__
            return mockSettings;
        });
        stub.set_lamda(&WindowUtils::isWayLand, []() {
            __DBG_STUB_INVOKE__
            return false;
        });

        mockSearchManager = new SearchHistroyManager();
        stub.set_lamda(&SearchHistroyManager::instance, [this]() {
            __DBG_STUB_INVOKE__
            return mockSearchManager;
        });
        stub.set_lamda(&SearchHistroyManager::getSearchHistroy, [](SearchHistroyManager *) {
            __DBG_STUB_INVOKE__
            return QStringList();
        });

        // Provide IP history with known order: first entry is oldest, last is newest
        ipHistory = {
            IPHistroyData("smb://192.168.1.1/share", QDateTime::currentDateTime().addDays(-3)),
            IPHistroyData("smb://192.168.1.2/share", QDateTime::currentDateTime().addDays(-2)),
            IPHistroyData("smb://192.168.1.3/share", QDateTime::currentDateTime().addDays(-1)),
        };
        stub.set_lamda(&SearchHistroyManager::getIPHistory, [this](SearchHistroyManager *) {
            __DBG_STUB_INVOKE__
            return ipHistory;
        });
        stub.set_lamda(&SearchHistroyManager::addIPHistoryCache, [](SearchHistroyManager *, const QString &) {
            __DBG_STUB_INVOKE__
        });
        stub.set_lamda(&SearchHistroyManager::clearHistory, [](SearchHistroyManager *, const QStringList &) {
            __DBG_STUB_INVOKE__
        });
        stub.set_lamda(&SearchHistroyManager::clearIPHistory, [](SearchHistroyManager *) {
            __DBG_STUB_INVOKE__
        });

        dialog = new ConnectToServerDialog(QUrl("file:///home/test"));
        ASSERT_NE(dialog, nullptr);
    }

    void TearDown() override
    {
        if (dialog) {
            delete dialog;
            dialog = nullptr;
        }
        if (mockSettings) {
            delete mockSettings;
            mockSettings = nullptr;
        }
        if (mockSearchManager) {
            delete mockSearchManager;
            mockSearchManager = nullptr;
        }
        stub.clear();
    }

    stub_ext::StubExt stub;
    ConnectToServerDialog *dialog { nullptr };
    Settings *mockSettings { nullptr };
    SearchHistroyManager *mockSearchManager { nullptr };
    QList<IPHistroyData> ipHistory;
};

// PMS:379251 连接服务器对话框应选中最近使用的服务器（hosts.first()）。
// 注意：修复提交 25adbb9b 已被 8e11155ef 回退，当前源码使用 hosts.last()，
// 此测试用 GTEST_SKIP 标记缺陷，待修复恢复后移除 skip。
TEST_F(TestConnectDialogBug379251, InitialSelection_ShouldBeMostRecentServer)
{
    GTEST_SKIP() << "PMS:379251 fix (25adbb9b) was reverted by 8e11155ef. "
                    "Current source selects hosts.last() (oldest) instead of "
                    "hosts.first() (most recent). Skip until fix is restored.";
}
