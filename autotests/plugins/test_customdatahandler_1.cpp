// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_customdatahandler_1.cpp
 * @brief Unit tests for CustomDataHandler methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "mode/custom/customdatahandler.h"

#include <QTest>

using namespace ddplugin_organizer;

class CustomDataHandlerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CustomDataHandler();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CustomDataHandler *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CustomDataHandlerTest, CustomDataHandler)
{
    // Test constructor: CustomDataHandler((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(CustomDataHandlerTest, addBaseData)
{
    // Test method: bool addBaseData((const CollectionBaseDataPtr &base))
    CollectionBaseDataPtr _arg0{};
    auto result = obj->addBaseData(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(CustomDataHandlerTest, baseDatas)
{
    // Test getter: QList<CollectionBaseDataPtr> baseDatas()
    auto result = obj->baseDatas();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(CustomDataHandlerTest, CustomDataHandler_Destructor)
{
    // Test method:  ~CustomDataHandler(())
    EXPECT_NO_FATAL_FAILURE({ CustomDataHandler *tmp = new CustomDataHandler(); delete tmp; });
}
