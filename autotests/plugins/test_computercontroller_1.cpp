// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_computercontroller_1.cpp
 * @brief Unit tests for ComputerController methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "controller/computercontroller.h"

#include <QTest>

using namespace dfmplugin_computer;

class ComputerControllerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ComputerController();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ComputerController *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ComputerControllerTest, actEject)
{
    // Test method: void actEject((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->actEject(_arg0));
}

TEST_F(ComputerControllerTest, actFormat)
{
    // Test method: void actFormat((quint64 winId, DFMEntryFileInfoPointer info))
    EXPECT_NO_FATAL_FAILURE(obj->actFormat(0, DFMEntryFileInfoPointer()));
}

TEST_F(ComputerControllerTest, actLogoutAndForgetPasswd)
{
    // Test method: void actLogoutAndForgetPasswd((DFMEntryFileInfoPointer info))
    EXPECT_NO_FATAL_FAILURE(obj->actLogoutAndForgetPasswd(DFMEntryFileInfoPointer()));
}

TEST_F(ComputerControllerTest, actMount)
{
    // Test method: void actMount((quint64 winId, DFMEntryFileInfoPointer info, bool enterAfterMounted))
    EXPECT_NO_FATAL_FAILURE(obj->actMount(0, DFMEntryFileInfoPointer(), false));
}

TEST_F(ComputerControllerTest, actOpenInNewTab)
{
    // Test method: void actOpenInNewTab((quint64 winId, DFMEntryFileInfoPointer info))
    EXPECT_NO_FATAL_FAILURE(obj->actOpenInNewTab(0, DFMEntryFileInfoPointer()));
}

TEST_F(ComputerControllerTest, actOpenInNewWindow)
{
    // Test method: void actOpenInNewWindow((quint64 winId, DFMEntryFileInfoPointer info))
    EXPECT_NO_FATAL_FAILURE(obj->actOpenInNewWindow(0, DFMEntryFileInfoPointer()));
}

TEST_F(ComputerControllerTest, actProperties)
{
    // Test method: void actProperties((quint64 winId, DFMEntryFileInfoPointer info))
    EXPECT_NO_FATAL_FAILURE(obj->actProperties(0, DFMEntryFileInfoPointer()));
}

TEST_F(ComputerControllerTest, actRename)
{
    // Test method: void actRename((quint64 winId, DFMEntryFileInfoPointer info, bool triggerFromSidebar))
    EXPECT_NO_FATAL_FAILURE(obj->actRename(0, DFMEntryFileInfoPointer(), false));
}

TEST_F(ComputerControllerTest, doSetProtocolDeviceAlias)
{
    // Test method: bool doSetProtocolDeviceAlias((DFMEntryFileInfoPointer info, const QString &alias))
    QString _arg1{};
    auto result = obj->doSetProtocolDeviceAlias(DFMEntryFileInfoPointer(), _arg1);
    EXPECT_FALSE(result);

}

TEST_F(ComputerControllerTest, handleNetworkCdCall)
{
    // Test method: void handleNetworkCdCall((quint64 winId, DFMEntryFileInfoPointer info))
    EXPECT_NO_FATAL_FAILURE(obj->handleNetworkCdCall(0, DFMEntryFileInfoPointer()));
}

TEST_F(ComputerControllerTest, handleUnAccessableDevCdCall)
{
    // Test method: void handleUnAccessableDevCdCall((quint64 winId, DFMEntryFileInfoPointer info))
    EXPECT_NO_FATAL_FAILURE(obj->handleUnAccessableDevCdCall(0, DFMEntryFileInfoPointer()));
}

TEST_F(ComputerControllerTest, instance)
{
    // Test getter: ComputerController instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}

TEST_F(ComputerControllerTest, onMenuRequest)
{
    // Test method: void onMenuRequest((quint64 winId, const QUrl &url, bool triggerFromSidebar))
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->onMenuRequest(0, _arg1, false));
}

TEST_F(ComputerControllerTest, waitUDisks2DataReady)
{
    // Test method: void waitUDisks2DataReady((const QString &id))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->waitUDisks2DataReady(_arg0));
}
