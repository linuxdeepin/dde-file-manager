// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_shredutils_1.cpp
 * @brief Unit tests for ShredUtils methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "shred/shredutils.h"

#include <QTest>

using namespace dfmplugin_utils;

class ShredUtilsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ShredUtils();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ShredUtils *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ShredUtilsTest, ShredUtils)
{
    // Test constructor: ShredUtils((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(ShredUtilsTest, confirmAndDisplayFiles)
{
    // Test method: bool confirmAndDisplayFiles((const QList<QUrl> &fileList))
    QList<QUrl> _arg0{};
    auto result = obj->confirmAndDisplayFiles(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(ShredUtilsTest, createShredSettingItem)
{
    // Test method: QWidget createShredSettingItem((QObject *opt))
    auto result = obj->createShredSettingItem(nullptr);
    EXPECT_NO_FATAL_FAILURE({ obj->createShredSettingItem(nullptr); });

}

TEST_F(ShredUtilsTest, initDconfig)
{
    // Test method: void initDconfig(())
    EXPECT_NO_FATAL_FAILURE(obj->initDconfig());
}

TEST_F(ShredUtilsTest, isShredEnabled)
{
    // Test bool getter: isShredEnabled()
    bool result = obj->isShredEnabled();
    EXPECT_FALSE(result);

}

TEST_F(ShredUtilsTest, setShredEnabled)
{
    // Test setter: void setShredEnabled((bool enable))
    EXPECT_NO_FATAL_FAILURE(obj->setShredEnabled(false));
}

TEST_F(ShredUtilsTest, shredfile)
{
    // Test method: void shredfile((const QList<QUrl> &fileList, quint64 winId))
    QList<QUrl> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->shredfile(_arg0, 0));
}

TEST_F(ShredUtilsTest, ShredUtils_Destructor)
{
    // Test method:  ~ShredUtils(())
    EXPECT_NO_FATAL_FAILURE({ ShredUtils *tmp = new ShredUtils(); delete tmp; });
}
