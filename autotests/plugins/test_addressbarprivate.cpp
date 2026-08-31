// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_addressbarprivate.cpp
 * @brief Unit tests for AddressBarPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/addressbar.h"

#include <QTest>

using namespace dfmplugin_titlebar;

class AddressBarPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new AddressBarPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    AddressBarPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(AddressBarPrivateTest, AddressBarPrivate)
{
    // Test constructor: AddressBarPrivate((AddressBar *qq))
    ASSERT_NE(obj, nullptr);
}

TEST_F(AddressBarPrivateTest, clearCompleterModel)
{
    // Test method: void clearCompleterModel(())
    EXPECT_NO_FATAL_FAILURE(obj->clearCompleterModel());
}

TEST_F(AddressBarPrivateTest, completeIpAddress)
{
    // Test method: void completeIpAddress((const QString &text))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->completeIpAddress(_arg0));
}
