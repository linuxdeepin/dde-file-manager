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
#include "Logger.h"

#define protected public
#define private public
#include "log/filterAppender.h"
#undef protected
#undef private


DCORE_USE_NAMESPACE
namespace  {
    class TestFilterAppender : public testing::Test
    {
    public:
        FilterAppender *m_appender;
        virtual void SetUp() override
        {
            m_appender = new FilterAppender();
            std::cout << "start TestFilterAppender" << std::endl;
        }

        virtual void TearDown() override
        {
            delete m_appender;
            m_appender = nullptr;
            std::cout << "end TestFilterAppender" << std::endl;
        }
    };
}

TEST_F(TestFilterAppender, data_pattern)
{
    m_appender->setDatePattern(FilterAppender::MinutelyRollover);
    EXPECT_EQ(FilterAppender::MinutelyRollover, m_appender->datePattern());

    QString pattern = "'.'yyyy-MM-dd-hh-mm-zzz";
    m_appender->setDatePattern(pattern);
    EXPECT_EQ(pattern, m_appender->datePatternString());
}

TEST_F(TestFilterAppender, log_file_limits)
{
    m_appender->setLogFilesLimit(1);
    EXPECT_EQ(1, m_appender->logFilesLimit());
}

TEST_F(TestFilterAppender, tst_filter)
{
    QString filter1 = "something1";
    QString filter2 = "something2";
    m_appender->addFilter(filter1);
    m_appender->addFilter(filter2);

    QStringList filters = m_appender->getFilters();
    EXPECT_TRUE(filters.size() > 0);
    EXPECT_EQ(filters.first(), filter1);

    m_appender->removeFilter(filter1);
    EXPECT_TRUE(m_appender->getFilters().size() == 1);

    m_appender->clearFilters();
    EXPECT_TRUE(m_appender->getFilters().size() == 0);
}

TEST_F(TestFilterAppender, tst_append)
{
    m_appender->setDatePattern(FilterAppender::HourlyRollover);
    m_appender->m_rollOverTime = QDateTime::fromTime_t(0);
    EXPECT_NO_FATAL_FAILURE(m_appender->append(QDateTime::currentDateTime(),
                                               Logger::Trace, "", 1, nullptr, "", "outputmessage"));

    m_appender->setDatePattern(FilterAppender::HalfDailyRollover);
    m_appender->m_rollOverTime = QDateTime::fromTime_t(0);
    EXPECT_NO_FATAL_FAILURE(m_appender->append(QDateTime::currentDateTime(),
                                               Logger::Trace, "", 1, nullptr, "", "outputmessage"));

    m_appender->setDatePattern(FilterAppender::DailyRollover);
    m_appender->m_rollOverTime = QDateTime::fromTime_t(0);
    EXPECT_NO_FATAL_FAILURE(m_appender->append(QDateTime::currentDateTime(),
                                               Logger::Trace, "", 1, nullptr, "", "outputmessage"));

    m_appender->setDatePattern(FilterAppender::WeeklyRollover);
    m_appender->m_rollOverTime = QDateTime::fromTime_t(0);
    EXPECT_NO_FATAL_FAILURE(m_appender->append(QDateTime::currentDateTime(),
                                               Logger::Trace, "", 1, nullptr, "", "outputmessage"));

    m_appender->setDatePattern(FilterAppender::MonthlyRollover);
    m_appender->m_rollOverTime = QDateTime::fromTime_t(0);
    EXPECT_NO_FATAL_FAILURE(m_appender->append(QDateTime::currentDateTime(),
                                               Logger::Trace, "", 1, nullptr, "", "outputmessage"));

    QString filter = "something";
    m_appender->addFilter(filter);

    EXPECT_NO_FATAL_FAILURE(m_appender->append(QDateTime::currentDateTime(),
                                               Logger::Trace, "", 1, nullptr, "", filter));
}

TEST_F(TestFilterAppender, remove_old_files)
{
    m_appender->setLogFilesLimit(-1);
    EXPECT_NO_FATAL_FAILURE(m_appender->removeOldFiles());

    m_appender->setLogFilesLimit(2);
    EXPECT_NO_FATAL_FAILURE(m_appender->removeOldFiles());
}

TEST_F(TestFilterAppender, tst_computeFrequency)
{
    EXPECT_NO_FATAL_FAILURE(m_appender->setDatePattern("'.'yyyy-MM-dd-hh-mm-zzz"));
    EXPECT_NO_FATAL_FAILURE(m_appender->setDatePattern("'.'yyyy-MM-dd-hh-mm"));
    EXPECT_NO_FATAL_FAILURE(m_appender->setDatePattern("'.'yyyy-MM-dd-hh"));
    EXPECT_NO_FATAL_FAILURE(m_appender->setDatePattern("'.'yyyy-MM-dd"));
    EXPECT_NO_FATAL_FAILURE(m_appender->setDatePattern("'.'yyyy-MM"));
}


