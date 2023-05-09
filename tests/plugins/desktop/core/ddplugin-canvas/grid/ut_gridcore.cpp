// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "grid/gridcore.h"

#include "stubext.h"

#include <gtest/gtest.h>

DDP_CANVAS_USE_NAMESPACE

TEST(GridCore, construct)
{
    GridCore core;
    core.surfaces.insert(1, QSize(10, 10));
    core.itemPos.insert(1, {});
    core.posItem.insert(1, {});
    core.overload.append("test");

    GridCore core2(core);
    EXPECT_EQ(core.surfaces, core2.surfaces);
    EXPECT_EQ(core.itemPos, core2.itemPos);
    EXPECT_EQ(core.posItem, core2.posItem);
    EXPECT_EQ(core.overload, core2.overload);
}

TEST(GridCore, surfaceSize)
{
    GridCore core;
    core.surfaces.insert(1, QSize(10, 10));
    EXPECT_EQ(core.surfaces.size(), 1);

    EXPECT_EQ(core.surfaceSize(1), QSize(10, 10));
    EXPECT_EQ(core.surfaceSize(2), QSize(0, 0));
}

TEST(GridCore, isValid)
{
    GridCore core;
    core.surfaces.insert(1, QSize(10, 10));

    EXPECT_TRUE(core.isValid(1, QPoint(2, 2)));
    EXPECT_FALSE(core.isValid(2, QPoint(2, 2)));
    EXPECT_FALSE(core.isValid(1, QPoint(11, 11)));
    EXPECT_FALSE(core.isValid(1, QPoint(-1, -1)));
}

TEST(GridCore, isVoid)
{
    GridCore core;
    core.posItem[1].insert(QPoint(0, 1), "test");
    EXPECT_FALSE(core.isVoid(1, QPoint(0, 1)));
    EXPECT_TRUE(core.isVoid(1, QPoint(2, 2)));
}

TEST(GridCore, pushOverload)
{
    GridCore core;
    core.pushOverload({QString("ss")});
    ASSERT_EQ(core.overload.size(), 1);
    EXPECT_EQ(core.overload.first(), QString("ss"));
}

TEST(GridCore, isFull)
{
    GridCore core;
    core.surfaces.insert(1, QSize(1, 2));

    EXPECT_FALSE(core.isFull(1));

    core.posItem[1].insert(QPoint(0,0), "");
    EXPECT_FALSE(core.isFull(1));

    core.posItem[1].insert(QPoint(0,1), "");
    EXPECT_TRUE(core.isFull(1));
}

class TestGridCore : public testing::Test
{
public:
    void SetUp() override {
        core.surfaces.insert(1, QSize(5, 5));
        core.surfaces.insert(2, QSize(9, 9));
        core.posItem[1].insert(QPoint(0, 1), QString("0,1"));
        core.posItem[1].insert(QPoint(1, 1), QString("1,1"));
        core.posItem[1].insert(QPoint(1, 2), QString("1,2"));
        core.itemPos[1].insert(QString("0,1"), QPoint(0, 1));
        core.itemPos[1].insert(QString("1,1"), QPoint(1, 1));
        core.itemPos[1].insert(QString("1,2"), QPoint(1, 2));

        core.posItem[2].insert(QPoint(0, 0), QString("0,0"));
        core.posItem[2].insert(QPoint(0, 2), QString("0,2"));
        core.posItem[2].insert(QPoint(0, 4), QString("0,4"));
        core.itemPos[2].insert(QString("0,0"), QPoint(0, 0));
        core.itemPos[2].insert(QString("0,2"), QPoint(0, 2));
        core.itemPos[2].insert(QString("0,4"), QPoint(0, 4));
    }

    GridCore core;
};

TEST_F(TestGridCore, surfaceIndex)
{
    auto list = core.surfaceIndex();
    ASSERT_EQ(list.size(), 2);
    EXPECT_EQ(list.first(), 1);
    EXPECT_EQ(list.last(), 2);
}

TEST_F(TestGridCore, applay)
{
    GridCore core2;
    EXPECT_FALSE(core2.applay(nullptr));
    EXPECT_TRUE(core2.surfaces.isEmpty());

    EXPECT_TRUE(core2.applay(&core));
    EXPECT_EQ(core.surfaces, core2.surfaces);
    EXPECT_EQ(core.itemPos, core2.itemPos);
    EXPECT_EQ(core.posItem, core2.posItem);
    EXPECT_EQ(core.overload, core2.overload);
}

TEST_F(TestGridCore, insert)
{
    core.insert(1, QPoint(2,2), QString("2,2"));
    EXPECT_EQ(core.itemPos[1].value(QString("2,2")), QPoint(2,2));
    EXPECT_EQ(core.posItem[1].value(QPoint(2,2)), QString("2,2"));

    core.insert(3, QPoint(2,2), QString("2,2"));
    EXPECT_EQ(core.itemPos[3].value(QString("2,2")), QPoint(2,2));
    EXPECT_EQ(core.posItem[3].value(QPoint(2,2)), QString("2,2"));
}

TEST_F(TestGridCore, remove)
{
    core.remove(1, QString(""));
    EXPECT_EQ(core.itemPos[1].size(), 3);
    EXPECT_EQ(core.posItem[1].size(), 3);

    core.remove(1, QPoint(3,3));
    EXPECT_EQ(core.itemPos[1].size(), 3);
    EXPECT_EQ(core.posItem[1].size(), 3);

    core.remove(1, QString("0,1"));
    EXPECT_FALSE(core.itemPos[1].contains(QString("0,1")));
    EXPECT_FALSE(core.posItem[1].contains(QPoint(0,1)));
    EXPECT_EQ(core.itemPos[1].size(), 2);
    EXPECT_EQ(core.posItem[1].size(), 2);

    core.remove(2, QString("0,0"));
    EXPECT_FALSE(core.itemPos[2].contains(QString("0,0")));
    EXPECT_FALSE(core.posItem[2].contains(QPoint(0,0)));
    EXPECT_EQ(core.itemPos[2].size(), 2);
    EXPECT_EQ(core.posItem[2].size(), 2);
}

TEST_F(TestGridCore, voidPos)
{
    auto pos = core.voidPos(1);
    EXPECT_EQ(pos.size(), 22);

    pos = core.voidPos(2);
    EXPECT_EQ(pos.size(), 78);
}

TEST_F(TestGridCore, findVoidPos)
{
    GridPos pos;
    EXPECT_TRUE(core.findVoidPos(pos));
    EXPECT_EQ(pos.first, 1);
    EXPECT_EQ(pos.second, QPoint(0,0));

    core.posItem[1].insert(QPoint(0, 0), QString("0,0"));
    EXPECT_TRUE(core.findVoidPos(pos));
    EXPECT_EQ(pos.first, 1);
    EXPECT_EQ(pos.second, QPoint(0,2));
}

TEST_F(TestGridCore, position)
{
    GridPos pos;

    EXPECT_FALSE(core.position(QString("2,3"), pos));

    EXPECT_TRUE(core.position(QString("0,2"), pos));
    EXPECT_EQ(pos.first, 2);
    EXPECT_EQ(pos.second, QPoint(0,2));

    EXPECT_TRUE(core.position(QString("0,1"), pos));
    EXPECT_EQ(pos.first, 1);
    EXPECT_EQ(pos.second, QPoint(0,1));

}

TEST_F(TestGridCore, item)
{
    GridPos pos;
    pos.first = 1;
    pos.second = QPoint(0,1);
    EXPECT_EQ(core.item(pos), QString("0,1"));

    pos.first = 2;
    pos.second = QPoint(0,0);
    EXPECT_EQ(core.item(pos), QString("0,0"));
}

TEST_F(TestGridCore, removeAll)
{
    core.overload.append("o");

    QStringList rd;
    rd.append("o");
    rd.append("0,0");
    rd.append("0,1");
    core.removeAll(rd);

    EXPECT_EQ(core.itemPos[1].size(), 2);
    EXPECT_EQ(core.posItem[1].size(), 2);
    EXPECT_EQ(core.itemPos[2].size(), 2);
    EXPECT_EQ(core.posItem[2].size(), 2);
    EXPECT_EQ(core.overload.size(), 0);

    EXPECT_FALSE(core.itemPos[1].contains(QString("0,1")));
    EXPECT_FALSE(core.posItem[1].contains(QPoint(0,1)));
    EXPECT_FALSE(core.itemPos[2].contains(QString("0,0")));
    EXPECT_FALSE(core.posItem[2].contains(QPoint(0,0)));
}

TEST_F(TestGridCore, move)
{
    MoveGridOper move(&core);

    QStringList list;
    list.append("0,1");
    list.append("1,1");

    GridPos pos;
    pos.first = 1;
    pos.second = QPoint(3,3);
    EXPECT_TRUE(move.move(pos, GridPos{1, QPoint(0,1)}, list));

    EXPECT_EQ(move.itemPos[1].value("0,1"), QPoint(3,3));
    EXPECT_EQ(move.itemPos[1].value("1,1"), QPoint(4,3));
}

TEST(AppendOper, tryAppendAfter)
{
    GridCore core;
    AppendOper ao(&core);
    ao.surfaces.insert(1, QSize(2, 2));

    QStringList list;
    list << "1" << "2" << "3" << "4" << "5";

    ao.tryAppendAfter(list, 1, QPoint(1, 0));
    EXPECT_EQ(ao.posItem[1].value(QPoint(1,0)), QString("1"));
    EXPECT_EQ(ao.posItem[1].value(QPoint(1,1)), QString("2"));
    EXPECT_EQ(ao.posItem[1].value(QPoint(0,0)), QString("3"));
    EXPECT_EQ(ao.posItem[1].value(QPoint(0,1)), QString("4"));
    EXPECT_TRUE(ao.overload.contains(QString("5")));
    EXPECT_EQ(ao.overload.size(), 1);
}
