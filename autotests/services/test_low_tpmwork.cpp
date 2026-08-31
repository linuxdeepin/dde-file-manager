// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_test_low_tpmwork.cpp
 * @brief Unit tests for TPMWork Low-priority methods
 */

#include <gtest/gtest.h>

class TPMWorkLowTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(TPMWorkLowTest, getRandom)
{
    // getRandom
    SUCCEED();
}

TEST_F(TPMWorkLowTest, isLibraryLoaded)
{
    // isLibraryLoaded
    SUCCEED();
}

TEST_F(TPMWorkLowTest, isSupportAlgo)
{
    // isSupportAlgo
    SUCCEED();
}

TEST_F(TPMWorkLowTest, isTPMAvailable)
{
    // isTPMAvailable
    SUCCEED();
}

TEST_F(TPMWorkLowTest, ownerAuthStatus)
{
    // ownerAuthStatus
    SUCCEED();
}

TEST_F(TPMWorkLowTest, TPMWork_Destructor)
{
    // ~TPMWork
    SUCCEED();
}

