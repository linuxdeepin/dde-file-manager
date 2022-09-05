// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include "logutil.h"
#include <DLog>

using namespace testing;
DCORE_USE_NAMESPACE

namespace  {
    class LogUtilTest : public Test
    {
    public:
    private:
        LogUtil *logUtil;
        virtual void SetUp() override{
            logUtil = new LogUtil();
        }

        virtual void TearDown() override{
            delete logUtil;
        }
    };

}

TEST_F(LogUtilTest, registerLogger)
{
    LogUtil::registerLogger();
}
