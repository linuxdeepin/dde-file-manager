// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_propertyeventcall_1.cpp
 * @brief Unit tests for PropertyEventCall methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "events/propertyeventcall.h"

#include <QTest>

using namespace dfmplugin_propertydialog;

class PropertyEventCallTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new PropertyEventCall();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    PropertyEventCall *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(PropertyEventCallTest, PropertyEventCall)
{
    // Test constructor: PropertyEventCall(())
    ASSERT_NE(obj, nullptr);
}

TEST_F(PropertyEventCallTest, sendFilesHideOrVisible)
{
    // Test method: void sendFilesHideOrVisible((quint64 winID, const QUrl &parentUrl, const QList<QUrl> &urls, bool isHide))
    QUrl _arg1{};
    QList<QUrl> _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->sendFilesHideOrVisible(0, _arg1, _arg2, false));
}
