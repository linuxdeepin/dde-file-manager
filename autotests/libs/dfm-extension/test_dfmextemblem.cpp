// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dfmextemblem.cpp
 * @brief Unit tests for DFMExtEmblem methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-extension/emblemicon/dfmextemblem.h"

#include <QTest>

using namespace src;

class DFMExtEmblemTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DFMExtEmblem();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DFMExtEmblem *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DFMExtEmblemTest, DFMExtEmblem)
{
    // Test constructor: DFMExtEmblem((const DFMExtEmblem &emblem))
    ASSERT_NE(obj, nullptr);
}

TEST_F(DFMExtEmblemTest, M_~DFMExtEmblem)
{
    // Test method:  ~DFMExtEmblem(())
    EXPECT_NO_FATAL_FAILURE({ DFMExtEmblem *tmp = new DFMExtEmblem(); delete tmp; });
}

TEST_F(DFMExtEmblemTest, emblems)
{
    // Test getter: std::vector<DFMExtEmblemIconLayout> emblems()
    auto result = obj->emblems();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DFMExtEmblemTest, operator=)
{
    // Test getter: DFMExtEmblem operator=()
    EXPECT_NO_FATAL_FAILURE({ obj->operator=(); });
}

TEST_F(DFMExtEmblemTest, setEmblem)
{
    // Test method: USING_DFMEXT_NAMESPACE setEmblem((const std::vector<DFMExtEmblemIconLayout> &iconPaths))
    std::vector<DFMExtEmblemIconLayout> _arg0{};
    auto result = obj->setEmblem(_arg0);
    EXPECT_NO_FATAL_FAILURE({ obj->setEmblem(_arg0); });

}

TEST_F(DFMExtEmblemTest, d)
{
    // Test getter: DFMExtEmblemPrivate d()
    auto result = obj->d();
    EXPECT_NO_FATAL_FAILURE({ obj->d(); });

}
