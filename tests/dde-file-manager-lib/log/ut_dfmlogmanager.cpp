// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include "QtTest/QTest"

#include "log/dfmLogManager.h"
#include "dfmstandardpaths.h"

DFM_USE_NAMESPACE
namespace  {
    class TestDFMLogManager : public testing::Test
    {
    public:
        virtual void SetUp() override
        {
            DFMLogManager::registerConsoleAppender();
            std::cout << "start TestDFMLogManager" << std::endl;
        }

        virtual void TearDown() override
        {
            std::cout << "end TestDFMLogManager" << std::endl;
        }
    };
}

TEST_F(TestDFMLogManager, set_log_path)
{
    QString strLogPath = DFMStandardPaths::location(DFMStandardPaths::CachePath) + "/ut.log";
    DFMLogManager::setlogFilePath(strLogPath);
    EXPECT_EQ(DFMLogManager::getlogFilePath(), strLogPath);
}


TEST_F(TestDFMLogManager, set_log_format)
{
    EXPECT_NO_FATAL_FAILURE(DFMLogManager::setLogFormat(""));
}

TEST_F(TestDFMLogManager, get_logger)
{
    EXPECT_NE(nullptr, DFMLogManager::getLogger());
}

TEST_F(TestDFMLogManager, get_appender)
{
    EXPECT_NE(nullptr, DFMLogManager::getFilterAppender());

    DFMLogManager::registerFileAppender();
    EXPECT_NE(nullptr, DFMLogManager::getFilterAppender());

}

