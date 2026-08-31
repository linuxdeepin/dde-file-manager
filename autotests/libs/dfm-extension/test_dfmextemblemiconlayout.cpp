// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dfmextemblemiconlayout.cpp
 * @brief Unit tests for DFMExtEmblemIconLayout methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-extension/emblemicon/dfmextemblemiconlayout.h"

#include <QTest>

using namespace src;

class DFMExtEmblemIconLayoutTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DFMExtEmblemIconLayout();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DFMExtEmblemIconLayout *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DFMExtEmblemIconLayoutTest, M_~DFMExtEmblemIconLayout)
{
    // Test method:  ~DFMExtEmblemIconLayout(())
    EXPECT_NO_FATAL_FAILURE({ DFMExtEmblemIconLayout *tmp = new DFMExtEmblemIconLayout(); delete tmp; });
}

TEST_F(DFMExtEmblemIconLayoutTest, DFMExtEmblemIconLayout)
{
    // Test constructor: DFMExtEmblemIconLayout((const DFMExtEmblemIconLayout &emblem))
    ASSERT_NE(obj, nullptr);
}

TEST_F(DFMExtEmblemIconLayoutTest, operator=)
{
    // Test getter: DFMExtEmblemIconLayout operator=()
    EXPECT_NO_FATAL_FAILURE({ obj->operator=(); });
}

TEST_F(DFMExtEmblemIconLayoutTest, x)
{
    // Test getter: int x()
    auto result = obj->x();
    EXPECT_EQ(result, 0);

}

TEST_F(DFMExtEmblemIconLayoutTest, y)
{
    // Test getter: int y()
    auto result = obj->y();
    EXPECT_EQ(result, 0);

}

TEST_F(DFMExtEmblemIconLayoutTest, iconPath)
{
    // Test getter: std::string iconPath()
    auto result = obj->iconPath();
    EXPECT_TRUE(result.empty());

}

TEST_F(DFMExtEmblemIconLayoutTest, LocationType)
{
    // Test method:  LocationType(())
    EXPECT_NO_FATAL_FAILURE(obj->LocationType());
}

TEST_F(DFMExtEmblemIconLayoutTest, d)
{
    // Test getter: DFMExtEmblemIconLayoutPrivate d()
    auto result = obj->d();
    EXPECT_NO_FATAL_FAILURE({ obj->d(); });

}
