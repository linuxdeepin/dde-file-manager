/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     dengkeyun<dengkeyun@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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

#include "models/searchhistory.h"

#include <gtest/gtest.h>

namespace {
class TestSearchHistory : public testing::Test
{
public:
    void SetUp() override
    {
        std::cout << "start TestSearchHistory";
        p = new SearchHistory;
    }

    void TearDown() override
    {
        std::cout << "end TestSearchHistory";
        delete p;
        p = nullptr;
    }

public:
    SearchHistory *p;
};
} // namespace

TEST_F(TestSearchHistory, searchDateTime)
{
    QDateTime dt = QDateTime::currentDateTime();
    p->setDateTime(dt);
    EXPECT_STREQ(dt.toString().toStdString().c_str(), p->getDateTime().toString().toStdString().c_str());
}

TEST_F(TestSearchHistory, searchKeyword)
{
    QString key = "test";
    p->setKeyword(key);
    EXPECT_STREQ("test", p->getKeyword().toStdString().c_str());
}

TEST_F(TestSearchHistory, tstConstructWithTime)
{
    QDateTime dt = QDateTime::currentDateTime();
    SearchHistory i(dt, "tst", nullptr);
    EXPECT_TRUE(dt == i.getDateTime());
    EXPECT_TRUE("tst" == i.getKeyword());
}
