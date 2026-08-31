// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_job_file_helper.cpp
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

TEST_F(job_file_helperTest, createEncryptJobFile)
{
    // Test method: int createEncryptJobFile((JobDescArgs &args))
    JobDescArgs _arg0{};
    auto result = obj->createEncryptJobFile(_arg0);
    EXPECT_GE(result, 0);

}

TEST_F(job_file_helperTest, hasJobFile)
{
    // Test bool getter: hasJobFile()
    bool result = obj->hasJobFile();
    EXPECT_FALSE(result);

}

TEST_F(job_file_helperTest, loadEncryptJobFile)
{
    // Test method: int loadEncryptJobFile((JobDescArgs *args, const QString &dev))
    QString _arg1{};
    auto result = obj->loadEncryptJobFile(nullptr, _arg1);
    EXPECT_GE(result, 0);

}

TEST_F(job_file_helperTest, removeJobFile)
{
    // Test method: int removeJobFile((const QString &jobFile))
    QString _arg0{};
    auto result = obj->removeJobFile(_arg0);
    EXPECT_GE(result, 0);

}
