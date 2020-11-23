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
    });

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
    });

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
    });

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
    });

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
    });

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
    });

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
    });

    const QList<float> ret = {1, 3.1, 3.2, 10000.1};
    EXPECT_TRUE(orgin.isEmpty());
    EXPECT_EQ(ret, out);
}

TEST(sortFunc,time_0)
{
    QMap<int, QList<float>> locate;
    for (float i = 0; i < 50; i += (rand() % 1 + 1)) {
        QList<float> el;
        for (float j = 0; j < 100; ++j)
            el << i + j / 100.0;
        locate.insert(i, el);
    }

    QList<float> orgin;
    for (float j = 0; j < 10; ++j)
        orgin << j;
    QList<float> out;

    timeval t1;
    timeval t2;
    gettimeofday(&t1, 0);
    DCustomActionDefines::sortFunc(locate, orgin, [&out](const QList<float> &data){
        for (auto it = data.begin(); it != data.end(); ++it)
            out.append(*it);
    });
    gettimeofday(&t2, 0);
    int ed = (t2.tv_sec - 1 - t1.tv_sec) * 1000000 + (t2.tv_usec - t1.tv_usec + 1000000);
    EXPECT_LE(ed, 1000);
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
    });

    const QList<float> ret = {1.1, 1.2, 3.1, 3.2, 2, 6.1, 6.2, 3, 4};
    EXPECT_TRUE(orgin.isEmpty());
    EXPECT_EQ(ret, out);
}
