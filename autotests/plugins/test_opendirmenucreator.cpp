// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_opendirmenucreator.cpp
 * @brief Unit tests for OpenDirMenuCreator methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menuscene/opendirmenuscene.h"

#include <QTest>

using namespace dfmplugin_menu;

class OpenDirMenuCreatorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new OpenDirMenuCreator();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    OpenDirMenuCreator *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(OpenDirMenuCreatorTest, create)
{
    // Test getter: DFMBASE_USE_NAMESPACE create()
    EXPECT_NO_FATAL_FAILURE({ obj->create(); });
}

TEST_F(OpenDirMenuCreatorTest, name)
{
    // Test getter: QString name()
    auto result = obj->name();
    EXPECT_TRUE(result.isEmpty());

}
