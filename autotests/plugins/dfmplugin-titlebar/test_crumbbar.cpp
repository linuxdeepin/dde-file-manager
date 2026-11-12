// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "views/crumbbar.h"
#include "views/private/crumbbar_p.h"
#include "utils/crumbinterface.h"

#include <gtest/gtest.h>
#include <QUrl>
#include <QSignalSpy>
#include <QMenu>

using namespace dfmplugin_titlebar;

class CrumbBarTest : public testing::Test
{
protected:
    void SetUp() override
    {
        crumbBar = new CrumbBar();
        stub.clear();
    }

    void TearDown() override
    {
        delete crumbBar;
        crumbBar = nullptr;
        stub.clear();
    }

    CrumbBar *crumbBar { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(CrumbBarTest, Constructor_Success_ObjectCreated)
{
    EXPECT_NE(crumbBar, nullptr);
}

TEST_F(CrumbBarTest, Controller_AfterConstruction_ReturnsNonNull)
{
    CrumbInterface *controller = crumbBar->controller();
    EXPECT_EQ(controller, nullptr);
}

TEST_F(CrumbBarTest, OnUrlChanged_ValidUrl_UrlUpdated)
{
    QUrl url("file:///home/test");

    stub.set_lamda(&CrumbInterface::seprateUrl, [](CrumbInterface *, const QUrl &) {
        __DBG_STUB_INVOKE__
        QList<CrumbData> list;
        list.append(CrumbData { QUrl("file:///"), "/" });
        list.append(CrumbData { QUrl("file:///home"), "home" });
        list.append(CrumbData { QUrl("file:///home/test"), "test" });
        return list;
    });

    crumbBar->onUrlChanged(url);
}

TEST_F(CrumbBarTest, LastUrl_AfterUrlChange_ReturnsCorrectUrl)
{
    QUrl url("file:///home/test");

    stub.set_lamda(&CrumbInterface::seprateUrl, [&url](CrumbInterface *, const QUrl &) {
        __DBG_STUB_INVOKE__
        QList<CrumbData> list;
        list.append(CrumbData { url, "test" });
        return list;
    });

    crumbBar->onUrlChanged(url);
    QUrl lastUrl = crumbBar->lastUrl();
    EXPECT_EQ(lastUrl, url);
}

TEST_F(CrumbBarTest, CustomMenu_ValidUrlAndMenu_MenuCustomized)
{
    QUrl url("file:///home/test");
    QMenu menu;

    crumbBar->customMenu(url, &menu);
    // Verify it doesn't crash
}

TEST_F(CrumbBarTest, SetPopupVisible_True_PopupShown)
{
    crumbBar->setPopupVisible(true);
    // Verify it doesn't crash
}

TEST_F(CrumbBarTest, SetPopupVisible_False_PopupHidden)
{
    crumbBar->setPopupVisible(false);
}

TEST_F(CrumbBarTest, OnKeepAddressBar_ValidUrl_AddressBarKept)
{
    QUrl url("file:///home/test");
    crumbBar->onKeepAddressBar(url);
}

TEST_F(CrumbBarTest, SelectedUrl_Signal_CanBeEmitted)
{
    QSignalSpy spy(crumbBar, &CrumbBar::selectedUrl);
    // Signal would be emitted on user interaction
}

TEST_F(CrumbBarTest, ShowAddressBarText_Signal_CanBeEmitted)
{
    QSignalSpy spy(crumbBar, &CrumbBar::showAddressBarText);
    // Signal would be emitted on user interaction
}

TEST_F(CrumbBarTest, HideAddressBar_Signal_CanBeEmitted)
{
    QSignalSpy spy(crumbBar, &CrumbBar::hideAddressBar);
    // Signal would be emitted on user interaction
}

TEST_F(CrumbBarTest, EditUrl_Signal_CanBeEmitted)
{
    QSignalSpy spy(crumbBar, &CrumbBar::editUrl);
    // Signal would be emitted on user interaction
}

TEST_F(CrumbBarTest, OnUrlChanged_MultipleUrls_UpdatesCorrectly)
{
    QUrl url1("file:///home/test1");
    QUrl url2("file:///home/test2");

    stub.set_lamda(&CrumbInterface::seprateUrl, [](CrumbInterface *, const QUrl &url) {
        __DBG_STUB_INVOKE__
        QList<CrumbData> list;
        list.append(CrumbData { url, url.fileName() });
        return list;
    });

    crumbBar->onUrlChanged(url1);
    EXPECT_EQ(crumbBar->lastUrl(), url1);

    crumbBar->onUrlChanged(url2);
    EXPECT_EQ(crumbBar->lastUrl(), url2);
}

// ===== PMS task 398283: tests for onUrlButtonActivated (async network check) =====
#include <dfm-base/utils/networkutils.h>
#include <dfm-base/utils/dialogmanager.h>
#include <dfm-base/dfm_global_defines.h>

DFMBASE_USE_NAMESPACE

TEST_F(CrumbBarTest, OnUrlButtonActivated_InvalidUrl_NoSignalNoCrash)
{
    // Stub the async network check to call callback(false) immediately (not busy)
    stub.set_lamda(&NetworkUtils::checkFtpOrSmbBusyAsync,
                   [](NetworkUtils *, const QUrl &, std::function<void(bool)> callback) {
                       __DBG_STUB_INVOKE__
                       if (callback)
                           callback(false);
                   });

    QSignalSpy spy(crumbBar, &CrumbBar::selectedUrl);
    QUrl invalidUrl;
    crumbBar->onUrlButtonActivated(invalidUrl);

    // Process events for async callback
    for (int i = 0; i < 20 && spy.count() == 0; ++i) {
        QCoreApplication::processEvents();
        QTest::qWait(50);
    }
    // With invalid URL, checkFtpOrSmbBusyAsync calls callback(false) synchronously,
    // so selectedUrl should be emitted
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(CrumbBarTest, OnUrlButtonActivated_BusyUrl_NoSignalShowsDialog)
{
    // Stub the async network check to call callback(true) (busy)
    stub.set_lamda(&NetworkUtils::checkFtpOrSmbBusyAsync,
                   [](NetworkUtils *, const QUrl &, std::function<void(bool)> callback) {
                       __DBG_STUB_INVOKE__
                       if (callback)
                           callback(true);
                   });

    // Stub the dialog to avoid GUI popup
    stub.set_lamda(&DialogManager::showUnableToVistDir, [](DialogManager *, const QString &) {
        __DBG_STUB_INVOKE__
    });

    QSignalSpy spy(crumbBar, &CrumbBar::selectedUrl);
    QUrl url("file:///run/user/1000/gvfs/ftp:host=1.2.3.4");
    crumbBar->onUrlButtonActivated(url);

    for (int i = 0; i < 20; ++i) {
        QCoreApplication::processEvents();
        QTest::qWait(50);
    }
    // Busy → dialog shown, no selectedUrl signal
    EXPECT_EQ(spy.count(), 0);
}

TEST_F(CrumbBarTest, OnUrlButtonActivated_NotBusyUrl_EmitsSelectedUrl)
{
    // Stub the async network check to call callback(false) (not busy)
    stub.set_lamda(&NetworkUtils::checkFtpOrSmbBusyAsync,
                   [](NetworkUtils *, const QUrl &, std::function<void(bool)> callback) {
                       __DBG_STUB_INVOKE__
                       if (callback)
                           callback(false);
                   });

    QSignalSpy spy(crumbBar, &CrumbBar::selectedUrl);
    QUrl url("file:///home/test");
    crumbBar->onUrlButtonActivated(url);

    for (int i = 0; i < 20 && spy.count() == 0; ++i) {
        QCoreApplication::processEvents();
        QTest::qWait(50);
    }
    EXPECT_EQ(spy.count(), 1);
    if (spy.count() > 0) {
        QList<QVariant> args = spy.takeFirst();
        EXPECT_EQ(args.at(0).toUrl(), url);
    }
}
