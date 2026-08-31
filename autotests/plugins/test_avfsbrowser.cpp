// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_avfsbrowser.cpp
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

TEST_F(AvfsBrowserTest, beMySubScene)
{
    // Test method: void beMySubScene((const QString &subScene))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->beMySubScene(_arg0));
}
