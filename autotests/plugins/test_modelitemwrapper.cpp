// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_modelitemwrapper.cpp
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

TEST_F(ModelItemWrapperTest, getData)
{
    // Test method: QVariant getData((int role))
    auto result = obj->getData(0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(ModelItemWrapperTest, isFileItem)
{
    // Test bool getter: isFileItem()
    bool result = obj->isFileItem();
    EXPECT_FALSE(result);

}

TEST_F(ModelItemWrapperTest, isGroupHeader)
{
    // Test bool getter: isGroupHeader()
    bool result = obj->isGroupHeader();
    EXPECT_FALSE(result);

}

TEST_F(ModelItemWrapperTest, isValid)
{
    // Test bool getter: isValid()
    bool result = obj->isValid();
    EXPECT_FALSE(result);

}
