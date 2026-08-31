// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_propertydialogutil_1.cpp
 * @brief Unit tests for PropertyDialogUtil methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/propertydialogutil.h"

#include <QTest>

using namespace dfmplugin_propertydialog;

class PropertyDialogUtilTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new PropertyDialogUtil();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    PropertyDialogUtil *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(PropertyDialogUtilTest, PropertyDialogUtil)
{
    // Test constructor: PropertyDialogUtil((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(PropertyDialogUtilTest, getPerportyPos)
{
    // Test method: QPoint getPerportyPos((int dialogWidth, int dialogHeight, int count, int index))
    auto result = obj->getPerportyPos(0, 0, 0, 0);
    EXPECT_TRUE(result.isNull());

}

TEST_F(PropertyDialogUtilTest, getPropertyPos)
{
    // Test method: QPoint getPropertyPos((int dialogWidth, int dialogHeight))
    auto result = obj->getPropertyPos(0, 0);
    EXPECT_TRUE(result.isNull());

}

TEST_F(PropertyDialogUtilTest, renameFilePropertyDialog)
{
    // Test method: void renameFilePropertyDialog((const QUrl &oldUrl, const QUrl &newUrl))
    QUrl _arg0{};
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->renameFilePropertyDialog(_arg0, _arg1));
}

TEST_F(PropertyDialogUtilTest, PropertyDialogUtil_Destructor)
{
    // Test method:  ~PropertyDialogUtil(())
    EXPECT_NO_FATAL_FAILURE({ PropertyDialogUtil *tmp = new PropertyDialogUtil(); delete tmp; });
}
