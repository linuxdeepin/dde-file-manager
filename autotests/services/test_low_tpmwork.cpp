// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_low_tpmwork.cpp
 * @brief Unit tests for TPMWork methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/tpmcontrol/core/tpmwork.h"

#include <QTest>

using namespace src;

class TPMWorkTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TPMWork();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TPMWork *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TPMWorkTest, getRandom)
{
    // Test method: int getRandom((int size, QString *output))
    auto result = obj->getRandom(0, nullptr);
    EXPECT_GE(result, 0);

}

TEST_F(TPMWorkTest, isLibraryLoaded)
{
    // Test bool getter: isLibraryLoaded()
    bool result = obj->isLibraryLoaded();
    EXPECT_FALSE(result);

}

TEST_F(TPMWorkTest, isSupportAlgo)
{
    // Test method: int isSupportAlgo((const QString &algoName, bool *support))
    QString _arg0{};
    auto result = obj->isSupportAlgo(_arg0, nullptr);
    EXPECT_GE(result, 0);

}

TEST_F(TPMWorkTest, isTPMAvailable)
{
    // Test getter: int isTPMAvailable()
    auto result = obj->isTPMAvailable();
    EXPECT_EQ(result, 0);

}

TEST_F(TPMWorkTest, ownerAuthStatus)
{
    // Test getter: int ownerAuthStatus()
    auto result = obj->ownerAuthStatus();
    EXPECT_EQ(result, 0);

}

TEST_F(TPMWorkTest, TPMWork_Destructor)
{
    // Test method:  ~TPMWork(())
    EXPECT_NO_FATAL_FAILURE({ TPMWork *tmp = new TPMWork(); delete tmp; });
}
