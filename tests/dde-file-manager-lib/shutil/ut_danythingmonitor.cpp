// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "shutil/danythingmonitor.h"

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

namespace  {
    class TestDAnythingMonitor : public testing::Test {
    public:
        void SetUp() override
        {
           mAnythingMonitor.reset(new DAnythingMonitor());
        }
        void TearDown() override
        {
        }

    public:
            std::shared_ptr<DAnythingMonitor> mAnythingMonitor = nullptr;
    };
}

TEST_F(TestDAnythingMonitor, prc_no_files_changed)
{
    mAnythingMonitor->workSignal();
    mAnythingMonitor->doWork();
}
