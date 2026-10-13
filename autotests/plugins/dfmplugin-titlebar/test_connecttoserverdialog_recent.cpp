// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <stubext.h>

#include <QCoreApplication>
#include <QTest>
#include <QUrl>
#include <QDateTime>
#include <QStringListModel>

#include <DComboBox>

#include "dialogs/connecttoserverdialog.h"
#include "utils/searchhistroymanager.h"

#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/settings.h>
#include <dfm-base/utils/windowutils.h>
#include <dfm-base/utils/dialogmanager.h>
#include <dfm-base/dfm_global_defines.h>

DFMBASE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
using namespace dfmplugin_titlebar;

class ConnectToServerRecentTest : public ::testing::Test
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

        mockSettings = new Settings("test_connectserver_recent");
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
        stub.set_lamda(&SearchHistroyManager::addIPHistoryCache, [](SearchHistroyManager *, const QString &) {
            __DBG_STUB_INVOKE__
        });
        stub.set_lamda(&SearchHistroyManager::clearHistory, [](SearchHistroyManager *, const QStringList &) {
            __DBG_STUB_INVOKE__
        });
        stub.set_lamda(&SearchHistroyManager::clearIPHistory, [](SearchHistroyManager *) {
            __DBG_STUB_INVOKE__
        });

        testUrl = QUrl("file:///home/test");
        dialog = nullptr;
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

    void setupIPHistory(const QList<IPHistroyData> &history)
    {
        stub.set_lamda(&SearchHistroyManager::getIPHistory,
                       [history](SearchHistroyManager *) {
                           __DBG_STUB_INVOKE__
                           return history;
                       });
    }

    stub_ext::StubExt stub;
    ConnectToServerDialog *dialog { nullptr };
    Settings *mockSettings { nullptr };
    SearchHistroyManager *mockSearchManager { nullptr };
    QUrl testUrl;
};

TEST_F(ConnectToServerRecentTest, InitServerDatas_SelectsMostRecentServer)
{
    QDateTime now = QDateTime::currentDateTime();
    QList<IPHistroyData> history;
    history << IPHistroyData("smb://192.168.1.100", now.addSecs(-3600));
    history << IPHistroyData("ftp://10.0.0.1", now.addSecs(-60));

    setupIPHistory(history);

    dialog = new ConnectToServerDialog(testUrl);
    ASSERT_NE(dialog, nullptr);

    // onCurrentInputChanged splits the URL: schemeComboBox gets "ftp://",
    // serverComboBox gets "10.0.0.1". Combine them for the full URL.
    QString scheme = dialog->schemeComboBox->currentText();
    QString server = dialog->serverComboBox->currentText();
    QString fullUrl = (scheme + server).toLower();
    EXPECT_EQ(fullUrl, QString("ftp://10.0.0.1"));
}

TEST_F(ConnectToServerRecentTest, InitServerDatas_MultipleServers_NewestSelectedFirst)
{
    QDateTime now = QDateTime::currentDateTime();
    QList<IPHistroyData> history;
    history << IPHistroyData("sftp://10.0.0.50", now.addSecs(-7200));
    history << IPHistroyData("smb://192.168.1.200", now.addSecs(-3600));
    history << IPHistroyData("ftp://10.0.0.1", now.addSecs(-30));

    setupIPHistory(history);

    dialog = new ConnectToServerDialog(testUrl);
    ASSERT_NE(dialog, nullptr);

    QString scheme = dialog->schemeComboBox->currentText();
    QString server = dialog->serverComboBox->currentText();
    QString fullUrl = (scheme + server).toLower();
    EXPECT_EQ(fullUrl, QString("ftp://10.0.0.1"));
}

TEST_F(ConnectToServerRecentTest, InitServerDatas_NoRecentHistory_ShowsPlaceholder)
{
    QDateTime old = QDateTime::currentDateTime().addDays(-30);
    QList<IPHistroyData> history;
    history << IPHistroyData("smb://192.168.1.100", old);

    setupIPHistory(history);

    dialog = new ConnectToServerDialog(testUrl);
    ASSERT_NE(dialog, nullptr);

    // Old entry (>7 days) is not recently accessed, so hosts is empty.
    // The dialog shows a "No history" placeholder item.
    int count = dialog->serverComboBox->count();
    EXPECT_EQ(count, 1);
}
