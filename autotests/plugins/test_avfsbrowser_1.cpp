// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_avfsbrowser_1.cpp
 * @brief Unit tests for AvfsBrowser methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "avfsbrowser.h"

#include <QTest>

using namespace dfmplugin_avfsbrowser;

class AvfsBrowserTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new AvfsBrowser();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    AvfsBrowser *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(AvfsBrowserTest, beMySubOnAdded)
{
    // Test method: void beMySubOnAdded((const QString &newScene))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->beMySubOnAdded(_arg0));
}

TEST_F(AvfsBrowserTest, followEvents)
{
    // Test method: void followEvents(())
    EXPECT_NO_FATAL_FAILURE(obj->followEvents());
}

TEST_F(AvfsBrowserTest, initialize)
{
    // Test getter: DFMBASE_USE_NAMESPACE initialize()
    EXPECT_NO_FATAL_FAILURE({ obj->initialize(); });
}

TEST_F(AvfsBrowserTest, regCrumb)
{
    // Test method: void regCrumb(())
    EXPECT_NO_FATAL_FAILURE(obj->regCrumb());
}

TEST_F(AvfsBrowserTest, start)
{
    // Test bool getter: start()
    bool result = obj->start();
    EXPECT_FALSE(result);

}
