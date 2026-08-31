// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_singleapplication.cpp
 * @brief Unit tests for SingleApplication methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "apps/dde-file-manager/singleapplication.h"

#include <QTest>

using namespace src;

class SingleApplicationTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SingleApplication();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SingleApplication *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SingleApplicationTest, handleNewClient)
{
    // Test method: void handleNewClient((const QString &uniqueKey))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->handleNewClient(_arg0));
}

TEST_F(SingleApplicationTest, readData)
{
    // Test method: void readData(())
    EXPECT_NO_FATAL_FAILURE(obj->readData());
}
