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

#include "models/bookmark.h"
#include "controllers/bookmarkmanager.h"
#include "dfmevent.h"

#include <gtest/gtest.h>

namespace {
class TestBookMark : public testing::Test
{
public:
    void SetUp() override
    {
        std::cout << "start TestBookMark\n";

        bm = new BookMark("Test", DUrl("file:///usr/bin"));
    }

    void TearDown() override
    {
        std::cout << "end TestBookMark\n";
        delete bm;
    }

public:
    BookMark *bm;
};
} // namespace

TEST_F(TestBookMark, tstConstructorWithoutName)
{
    BookMark b(DUrl("/home"));
    EXPECT_TRUE(b.exists());
}

TEST_F(TestBookMark, tstBasicProperties)
{
    auto u = bm->sourceUrl();
    EXPECT_TRUE(u.path() == "/usr/bin");

    auto name = bm->fileDisplayName();
    EXPECT_TRUE(name == "Test");

    auto mpt = bm->getMountPoint();
    EXPECT_FALSE(!mpt.isEmpty());

    EXPECT_TRUE(bm->canRedirectionFileUrl());

    u = bm->redirectedFileUrl();
    EXPECT_FALSE(!u.isValid());

    u = bm->parentUrl();
    EXPECT_FALSE(!u.isValid());

    auto dt = bm->created();
    EXPECT_FALSE(dt.isValid());
    dt = bm->lastModified();
    EXPECT_FALSE(dt.isValid());
}

