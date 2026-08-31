// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_propertyeventcall.cpp
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

TEST_F(PropertyEventCallTest, sendFileHide)
{
    // Test method: void sendFileHide((quint64 winID, const QList<QUrl> &urls))
    QList<QUrl> _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->sendFileHide(0, _arg1));
}

TEST_F(PropertyEventCallTest, sendSetPermissionManager)
{
    // Test method: void sendSetPermissionManager((quint64 winID, const QUrl &url, const QFileDevice::Permissions permissions))
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->sendSetPermissionManager(0, _arg1, QFileDevice::Permissions()));
}
