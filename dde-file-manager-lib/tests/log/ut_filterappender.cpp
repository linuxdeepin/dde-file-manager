#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include "QtTest/QTest"

#include "log/filterAppender.h"


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
    QString filter = "something";
    m_appender->addFilter(filter);

    QStringList filters = m_appender->getFilters();
    EXPECT_TRUE(filters.size() > 0);
    EXPECT_EQ(filters.first(), filter);

    m_appender->clearFilters();
    EXPECT_TRUE(m_appender->getFilters().size() == 0);
}


