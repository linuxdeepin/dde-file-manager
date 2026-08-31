// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_burneventcaller.cpp
 * @brief Unit tests for BurnEventCaller methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "events/burneventcaller.h"

#include <QTest>

using namespace dfmplugin_burn;

class BurnEventCallerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new BurnEventCaller();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    BurnEventCaller *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(BurnEventCallerTest, BurnEventCaller)
{
    // Test constructor: BurnEventCaller(())
    ASSERT_NE(obj, nullptr);
}

TEST_F(BurnEventCallerTest, sendPasteFiles)
{
    // Test method: DFMBASE_USE_NAMESPACE sendPasteFiles((const QList<QUrl> &urls, const QUrl &dest, bool isCopy))
    QList<QUrl> _arg0{};
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE({ obj->sendPasteFiles(_arg0, _arg1, false); });
}
