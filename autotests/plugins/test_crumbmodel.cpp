// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_crumbmodel.cpp
 * @brief Unit tests for CrumbModel methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "models/crumbmodel.h"

#include <QTest>

using namespace dfmplugin_titlebar;

class CrumbModelTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CrumbModel();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CrumbModel *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CrumbModelTest, lastIndex)
{
    // Test getter: QModelIndex lastIndex()
    auto result = obj->lastIndex();
    EXPECT_FALSE(result.isValid());

}

TEST_F(CrumbModelTest, removeAll)
{
    // Test method: void removeAll(())
    EXPECT_NO_FATAL_FAILURE(obj->removeAll());
}
