/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

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

