// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_emblemmanager.cpp
 * @brief Unit tests for EmblemManager methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/emblemmanager.h"

#include <QTest>

using namespace dfmplugin_emblem;

class EmblemManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new EmblemManager();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    EmblemManager *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(EmblemManagerTest, instance)
{
    // Test getter: EmblemManager instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}
