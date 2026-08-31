// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_modelitemwrapper_1.cpp
 * @brief Unit tests for ModelItemWrapper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "groups/modelitemwrapper.h"

#include <QTest>

using namespace dfmplugin_workspace;

class ModelItemWrapperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ModelItemWrapper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ModelItemWrapper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ModelItemWrapperTest, ModelItemWrapper)
{
    // Test constructor: ModelItemWrapper((const ModelItemWrapper &other))
    ASSERT_NE(obj, nullptr);
}

TEST_F(ModelItemWrapperTest, M_~ModelItemWrapper)
{
    // Test method:  ~ModelItemWrapper(())
    EXPECT_NO_FATAL_FAILURE({ ModelItemWrapper *tmp = new ModelItemWrapper(); delete tmp; });
}

TEST_F(ModelItemWrapperTest, operator=)
{
    // Test getter: ModelItemWrapper operator=()
    EXPECT_NO_FATAL_FAILURE({ obj->operator=(); });
}

TEST_F(ModelItemWrapperTest, getData)
{
    // Test method: QVariant getData((int role))
    auto result = obj->getData(0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(ModelItemWrapperTest, isValid)
{
    // Test bool getter: isValid()
    bool result = obj->isValid();
    EXPECT_FALSE(result);

}

TEST_F(ModelItemWrapperTest, isGroupHeader)
{
    // Test bool getter: isGroupHeader()
    bool result = obj->isGroupHeader();
    EXPECT_FALSE(result);

}

TEST_F(ModelItemWrapperTest, isFileItem)
{
    // Test bool getter: isFileItem()
    bool result = obj->isFileItem();
    EXPECT_FALSE(result);

}

TEST_F(ModelItemWrapperTest, ItemType)
{
    // Test method:  ItemType(())
    EXPECT_NO_FATAL_FAILURE(obj->ItemType());
}

TEST_F(ModelItemWrapperTest, itemType)
{
    // Test getter: ItemType itemType()
    auto result = obj->itemType();
    EXPECT_NO_FATAL_FAILURE({ obj->itemType(); });

}

TEST_F(ModelItemWrapperTest, groupKey)
{
    // Test getter: QString groupKey()
    auto result = obj->groupKey();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(ModelItemWrapperTest, fileData)
{
    // Test getter: FileItemDataPointer fileData()
    auto result = obj->fileData();
    EXPECT_EQ(result.get(), nullptr);

}

TEST_F(ModelItemWrapperTest, groupValues)
{
    // Test getter: QHash<int, QVariant> groupValues()
    auto result = obj->groupValues();
    EXPECT_TRUE(result.isEmpty());

}
