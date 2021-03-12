/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: dengkeyun<dengkeyun@uniontech.com>
 *             max-lv<lvwujun@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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
#include "mediainfo/dfmmediainfo.h"

DFM_USE_NAMESPACE

namespace {
class TestDFMMediaInfo : public testing::Test
{
public:
    void SetUp() override
    {
        std::cout << "start TestDFMMediaInfo";
        info = new DFMMediaInfo("test.mp3", nullptr);
    }

    void TearDown() override
    {
        std::cout << "end TestDFMMediaInfo";
        delete info;
        info = nullptr;
    }

public:
    DFMMediaInfo *info = nullptr;
};
} // namespace

TEST_F(TestDFMMediaInfo, StringPropertyTest)
{
    info->startReadInfo();
    QThread::msleep(200);
    EXPECT_STREQ("", info->Value("").toStdString().c_str());
}
