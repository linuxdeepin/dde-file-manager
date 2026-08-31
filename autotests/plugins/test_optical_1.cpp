// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_optical_1.cpp
 * @brief Unit tests for Optical methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "optical.h"

#include <QTest>

using namespace dfmplugin_optical;

class OpticalTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new Optical();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    Optical *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(OpticalTest, addCustomTopWidget)
{
    // Test method: void addCustomTopWidget(())
    EXPECT_NO_FATAL_FAILURE(obj->addCustomTopWidget());
}

TEST_F(OpticalTest, addDelegateSettings)
{
    // Test method: void addDelegateSettings(())
    EXPECT_NO_FATAL_FAILURE(obj->addDelegateSettings());
}

TEST_F(OpticalTest, addOpticalCrumbToTitleBar)
{
    // Test method: void addOpticalCrumbToTitleBar(())
    EXPECT_NO_FATAL_FAILURE(obj->addOpticalCrumbToTitleBar());
}

TEST_F(OpticalTest, addPropertySettings)
{
    // Test method: void addPropertySettings(())
    EXPECT_NO_FATAL_FAILURE(obj->addPropertySettings());
}

TEST_F(OpticalTest, bindFileOperations)
{
    // Test method: void bindFileOperations(())
    EXPECT_NO_FATAL_FAILURE(obj->bindFileOperations());
}

TEST_F(OpticalTest, bindWindows)
{
    // Test method: void bindWindows(())
    EXPECT_NO_FATAL_FAILURE(obj->bindWindows());
}

TEST_F(OpticalTest, changeUrlEventFilter)
{
    // Test method: bool changeUrlEventFilter((quint64 windowId, const QUrl &url))
    QUrl _arg1{};
    auto result = obj->changeUrlEventFilter(0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(OpticalTest, initialize)
{
    // Test method: void initialize(())
    EXPECT_NO_FATAL_FAILURE(obj->initialize());
}

TEST_F(OpticalTest, onAllPluginsStarted)
{
    // Test method: void onAllPluginsStarted(())
    EXPECT_NO_FATAL_FAILURE(obj->onAllPluginsStarted());
}

TEST_F(OpticalTest, onDiscChanged)
{
    // Test method: void onDiscChanged((const QString &id))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onDiscChanged(_arg0));
}

TEST_F(OpticalTest, onDiscEjected)
{
    // Test method: void onDiscEjected((const QString &id))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onDiscEjected(_arg0));
}

TEST_F(OpticalTest, openNewWindowEventFilter)
{
    // Test method: bool openNewWindowEventFilter((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->openNewWindowEventFilter(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(OpticalTest, openNewWindowWithArgsEventFilter)
{
    // Test method: bool openNewWindowWithArgsEventFilter((const QUrl &url, bool isNewWindow))
    QUrl _arg0{};
    auto result = obj->openNewWindowWithArgsEventFilter(_arg0, false);
    EXPECT_FALSE(result);

}

TEST_F(OpticalTest, packetWritingUrl)
{
    // Test method: bool packetWritingUrl((const QUrl &srcUrl, QUrl *url))
    QUrl _arg0{};
    auto result = obj->packetWritingUrl(_arg0, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(OpticalTest, start)
{
    // Test bool getter: start()
    bool result = obj->start();
    EXPECT_FALSE(result);

}
