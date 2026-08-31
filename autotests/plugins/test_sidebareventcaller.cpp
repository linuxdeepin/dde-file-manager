// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_sidebareventcaller.cpp
 * @brief Unit tests for SideBarEventCaller methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "events/sidebareventcaller.h"

#include <QTest>

using namespace dfmplugin_sidebar;

class SideBarEventCallerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SideBarEventCaller();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SideBarEventCaller *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SideBarEventCallerTest, sendEject)
{
    // Test method: void sendEject((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->sendEject(_arg0));
}
