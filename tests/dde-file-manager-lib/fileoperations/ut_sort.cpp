/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liyigang<liyigang@uniontech.com>
 *
 * Maintainer: liyigang<liyigang@uniontech.com>
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
#include "fileoperations/sort.h"
#include "testhelper.h"
#include "stub.h"

#include <QString>
#include <QProcess>


using namespace testing;


TEST(savedirTest,AnyAgruInput){
    Stub stl;
    TestHelper::runInLoop([](){});

    direntry_t stl1,stl2;
    char tt[10] = {'a'};
    stl1.name = tt;
    stl1.ino = 9;
    stl2.ino = 10;
    stl2.name = tt;
    EXPECT_TRUE(direntry_cmp_inode(&stl1,&stl2));
    EXPECT_TRUE(direntry_cmp_inode(&stl1,&stl2));
    EXPECT_TRUE(QString(savedir("~/Pictures")).isNull());
    QProcess::execute("mkdir /tmp/test_temp_sourt");
    QProcess::execute("chmode 0000 /tmp/test_temp_sourt");
    EXPECT_FALSE(QString(savedir("/tmp/test_temp_sourt")).isNull());
    EXPECT_FALSE(QString(savedir("/tmp")).isNull());
    EXPECT_FALSE(QString(savedir("/usr")).isNull());
    int (*closedir1)(DIR *) = [](DIR *){return -1;};
    stl.set(closedir,closedir1);
    EXPECT_TRUE(QString(savedir("/tmp/test_temp_sourt")).isNull());
    QProcess::execute("chmode 0700 /tmp/test_temp_sourt");
    QProcess::execute("rm -r /tmp/test_temp_sourt");
}
