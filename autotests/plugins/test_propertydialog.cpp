// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_propertydialog.cpp
 * @brief Unit tests for PropertyDialog methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "propertydialog.h"

#include <QTest>

using namespace dfmplugin_propertydialog;

class PropertyDialogTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new PropertyDialog();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    PropertyDialog *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(PropertyDialogTest, start)
{
    // Test bool getter: start()
    bool result = obj->start();
    EXPECT_FALSE(result);

}
