// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include "customization/dcustomactiondefine.h"

#include <QDebug>

#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>

TEST(sortFunc,empty_0)
{
    QMap<int, QList<float>> locate;
    locate.insert(1,{1.1,1.2,1.3});
    locate.insert(2,{2.1});
    locate.insert(4,{});
    locate.insert(5,{5.1});
    QList<float> orgin;
    QList<float> out;

    DCustomActionDefines::sortFunc(locate, orgin, [&out](const QList<float> &data){
        for (auto it = data.begin(); it != data.end(); ++it)
            out.append(*it);
    },[](float){return true;});

    const QList<float> ret = {1.1,1.2,1.3,2.1,5.1};
    EXPECT_TRUE(orgin.isEmpty());
    EXPECT_EQ(ret, out);
}

TEST(sortFunc,empty_1)
{
    QMap<int, QList<float>> locate;

    QList<float> orgin = {1.1,2.2,3.3};
    QList<float> out;
    const QList<float> ret = orgin;
    DCustomActionDefines::sortFunc(locate, orgin, [&out](const QList<float> &data){
        for (auto it = data.begin(); it != data.end(); ++it)
            out.append(*it);
    },[](float){return true;});

    EXPECT_TRUE(orgin.isEmpty());
    EXPECT_EQ(ret, out);
}

TEST(sortFunc,norml_0)
{
    QMap<int, QList<float>> locate;
    locate.insert(1,{1.1,1.2,1.3});
    QList<float> orgin = {1,2,3,4};
    QList<float> out;

    DCustomActionDefines::sortFunc(locate, orgin, [&out](const QList<float> &data){
        for (auto it = data.begin(); it != data.end(); ++it)
            out.append(*it);
    },[](float){return true;});

    const QList<float> ret = {1.1, 1.2, 1.3, 1, 2, 3, 4};
    EXPECT_TRUE(orgin.isEmpty());
    EXPECT_EQ(ret, out);
}

TEST(sortFunc,norml_1)
{
    QMap<int, QList<float>> locate;
    locate.insert(3,{3.1,3.2});
    QList<float> orgin = {1,2,3,4};
    QList<float> out;

    DCustomActionDefines::sortFunc(locate, orgin, [&out](const QList<float> &data){
        for (auto it = data.begin(); it != data.end(); ++it)
            out.append(*it);
    },[](float){return true;});

    const QList<float> ret = {1, 2, 3.1, 3.2, 3, 4};
    EXPECT_TRUE(orgin.isEmpty());
    EXPECT_EQ(ret, out);
}

TEST(sortFunc,norml_2)
{
    QMap<int, QList<float>> locate;
    locate.insert(3,{3.1,3.2});
    QList<float> orgin = {1,2,3};
    QList<float> out;

    DCustomActionDefines::sortFunc(locate, orgin, [&out](const QList<float> &data){
        for (auto it = data.begin(); it != data.end(); ++it)
            out.append(*it);
    },[](float){return true;});

    const QList<float> ret = {1, 2, 3.1, 3.2, 3};
    EXPECT_TRUE(orgin.isEmpty());
    EXPECT_EQ(ret, out);
}

TEST(sortFunc,norml_3)
{
    QMap<int, QList<float>> locate;
    locate.insert(3,{3.1,3.2});
    locate.insert(999,{999.1,999.2});
    QList<float> orgin = {1,2,3,4};
    QList<float> out;

    DCustomActionDefines::sortFunc(locate, orgin, [&out](const QList<float> &data){
        for (auto it = data.begin(); it != data.end(); ++it)
            out.append(*it);
    },[](float){return true;});

    const QList<float> ret = {1, 2, 3.1, 3.2, 3, 4, 999.1, 999.2};
    EXPECT_TRUE(orgin.isEmpty());
    EXPECT_EQ(ret, out);
}

TEST(sortFunc,cornor_0)
{
    QMap<int, QList<float>> locate;
    locate.insert(3, {3.1,3.2});
    locate.insert(10000,{10000.1});
    QList<float> orgin = {1};
    QList<float> out;

    DCustomActionDefines::sortFunc(locate, orgin, [&out](const QList<float> &data){
        for (auto it = data.begin(); it != data.end(); ++it)
            out.append(*it);
    },[](float){return true;});

    const QList<float> ret = {1, 3.1, 3.2, 10000.1};
    EXPECT_TRUE(orgin.isEmpty());
    EXPECT_EQ(ret, out);
}

TEST(sortFunc,norml_4)
{
    QMap<int, QList<float>> locate;
    locate.insert(1,{1.1, 1.2});
    locate.insert(3,{3.1, 3.2});
    locate.insert(6,{6.1, 6.2});
    QList<float> orgin = {2, 3, 4};
    QList<float> out;

    DCustomActionDefines::sortFunc(locate, orgin, [&out](const QList<float> &data){
        for (auto it = data.begin(); it != data.end(); ++it)
            out.append(*it);
    },[](float){return true;});

    const QList<float> ret = {1.1, 1.2, 3.1, 3.2, 2, 6.1, 6.2, 3, 4};
    EXPECT_TRUE(orgin.isEmpty());
    EXPECT_EQ(ret, out);
}

TEST(sortFunc,filter_0)
{
    QMap<int, QList<float>> locate;
    locate.insert(1,{1.1, 1.2});
    locate.insert(4,{4.1, 4.2});
    locate.insert(7,{7.1, 7.2});
    QList<float> orgin = {0, 0, 2, 0, 3, 4};
    QList<float> out;

    DCustomActionDefines::sortFunc(locate, orgin, [&out](const QList<float> &data){
        for (auto it = data.begin(); it != data.end(); ++it)
            out.append(*it);
    },[](float f){return f != 0;});

    const QList<float> ret = {1.1, 1.2, 0, 0, 2, 4.1, 4.2, 0, 3, 7.1, 7.2, 4};
    EXPECT_TRUE(orgin.isEmpty());
    EXPECT_EQ(ret, out);
}

TEST(sortFunc,filter_1)
{
    QMap<int, QList<float>> locate;
    locate.insert(4,{4.1, 4.2});
    QList<float> orgin = {0, 0, 2};
    QList<float> out;

    DCustomActionDefines::sortFunc(locate, orgin, [&out](const QList<float> &data){
        for (auto it = data.begin(); it != data.end(); ++it)
            out.append(*it);
    },[](float f){return f != 0;});

    const QList<float> ret = {0, 0, 2, 4.1, 4.2};
    EXPECT_TRUE(orgin.isEmpty());
    EXPECT_EQ(ret, out);
}
