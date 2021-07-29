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

#define Free(x) if(x){free(x);x=nullptr;}

using namespace testing;

#ifndef __arm__
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
    char* name_space = savedir("~/Pictures");
    EXPECT_TRUE(QString(name_space).isNull());
    Free(name_space);

    QProcess::execute("mkdir /tmp/test_temp_sourt");
    QProcess::execute("chmode 0000 /tmp/test_temp_sourt");
    name_space = savedir("/tmp/test_temp_sourt");
    EXPECT_FALSE(QString(name_space).isNull());
    Free(name_space);

    name_space = savedir("/tmp");
    EXPECT_FALSE(QString(name_space).isNull());
    Free(name_space);

    name_space = savedir("/usr");
    EXPECT_FALSE(QString(name_space).isNull());
    Free(name_space);

    int (*closedir1)(DIR *) = [](DIR *){return -1;};
    stl.set(closedir,closedir1);

    name_space = savedir("/tmp/test_temp_sourt");
    EXPECT_TRUE(QString(name_space).isNull());
    QProcess::execute("chmode 0700 /tmp/test_temp_sourt");
    QProcess::execute("rm -r /tmp/test_temp_sourt");

    Free(name_space);
}
#endif
