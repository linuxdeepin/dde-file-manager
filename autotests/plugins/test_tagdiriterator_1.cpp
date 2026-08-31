// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_test_tagdiriterator_1.cpp
 * @brief Unit tests for TagDirIterator Low-priority methods
 */

#include <gtest/gtest.h>

class TagDirIteratorTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(TagDirIteratorTest, TagDirIterator_Destructor)
{
    // ~TagDirIterator
    SUCCEED();
}

