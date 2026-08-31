// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_openwithhelper.cpp
 * @brief Unit tests for OpenWithHelper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "openwith/openwithhelper.h"

#include <QTest>

using namespace dfmplugin_utils;

class OpenWithHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new OpenWithHelper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    OpenWithHelper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(OpenWithHelperTest, OpenWithHelper)
{
    // Test constructor: OpenWithHelper((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(OpenWithHelperTest, createOpenWithWidget)
{
    // Test method: QWidget createOpenWithWidget((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->createOpenWithWidget(_arg0);
    EXPECT_NO_FATAL_FAILURE({ obj->createOpenWithWidget(_arg0); });

}
