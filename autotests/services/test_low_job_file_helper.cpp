// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_low_job_file_helper.cpp
 * @brief Unit tests for job_file_helper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/diskencrypt/helpers/jobfilehelper.h"

#include <QTest>

using namespace src;

class job_file_helperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new job_file_helper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    job_file_helper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(job_file_helperTest, createDecryptJobFile)
{
    // Test method: int createDecryptJobFile((JobDescArgs &args))
    JobDescArgs _arg0{};
    auto result = obj->createDecryptJobFile(_arg0);
    EXPECT_GE(result, 0);

}

TEST_F(job_file_helperTest, createUSecRoot)
{
    // Test getter: int createUSecRoot()
    auto result = obj->createUSecRoot();
    EXPECT_EQ(result, 0);

}

TEST_F(job_file_helperTest, validJobTypes)
{
    // Test getter: QStringList validJobTypes()
    auto result = obj->validJobTypes();
    EXPECT_TRUE(result.isEmpty());

}
