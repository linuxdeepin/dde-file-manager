// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "grid/canvasgrid_p.h"
#include "displayconfig.h"

#include "stubext.h"

#include <gtest/gtest.h>

DDP_CANVAS_USE_NAMESPACE

TEST(CanvasGrid, initSurface)
{
    CanvasGrid grid;
    EXPECT_TRUE(grid.d->syncTimer.isSingleShot());
    grid.d->posItem.insert(1, {});
    grid.d->itemPos.insert(1, {});
    grid.d->overload.append("1");
    grid.d->surfaces.insert(3, QSize(2,2));

    grid.initSurface(2);
    EXPECT_TRUE(grid.d->posItem.isEmpty());
    EXPECT_TRUE(grid.d->itemPos.isEmpty());
    EXPECT_TRUE(grid.d->overload.isEmpty());
    EXPECT_TRUE(grid.d->surfaces.contains(1));
    EXPECT_TRUE(grid.d->surfaces.contains(2));
}

TEST(CanvasGrid, updateSize)
{
    CanvasGrid grid;
    grid.initSurface(2);

    QStringList items;
    stub_ext::StubExt stub;
    stub.set_lamda(&CanvasGrid::items, [&items](){
        return items;
    });

    QStringList outItems;
    stub.set_lamda(&CanvasGrid::setItems, [&outItems](CanvasGrid *,
                   const QStringList &items){
        outItems = items;
        return;
    });

    items << "1";

    {
        grid.d->itemPos[2].insert("1", QPoint(0,0));

        grid.updateSize(1, QSize(2, 2));
        EXPECT_TRUE(outItems.isEmpty());
        EXPECT_EQ(grid.d->surfaces.value(1), QSize(2, 2));
        EXPECT_EQ(grid.d->surfaces.value(2), QSize(0, 0));

        grid.updateSize(2, QSize(3, 2));
        EXPECT_EQ(grid.d->surfaces.value(1), QSize(2, 2));
        EXPECT_EQ(grid.d->surfaces.value(2), QSize(3, 2));
        ASSERT_EQ(outItems.size(), 1);
    }
}

TEST(CanvasGrid, gridCount)
{
    CanvasGrid grid;
    grid.d->surfaces.insert(1, QSize(2,2));
    grid.d->surfaces.insert(2, QSize(2,3));

    EXPECT_EQ(grid.gridCount(3), 0);
    EXPECT_EQ(grid.gridCount(1), 4);
    EXPECT_EQ(grid.gridCount(2), 6);
    EXPECT_EQ(grid.gridCount(), 10);
}

TEST(CanvasGrid, setItems)
{
    stub_ext::StubExt stub;
    CanvasGrid grid;
    QStringList item1;
    stub.set_lamda(&CanvasGridPrivate::restore, [&item1](CanvasGridPrivate *,
                   QStringList currentItems){
        item1 = currentItems;
    });

    QStringList item2;
    stub.set_lamda(&CanvasGridPrivate::sequence, [&item2](CanvasGridPrivate *,
                   QStringList currentItems){
        item2 = currentItems;
    });

    QStringList in{QString("1"), QString("2")};
    grid.d->mode = CanvasGrid::Mode::Custom;

    grid.setItems(in);
    EXPECT_EQ(item1, in);
    EXPECT_TRUE(item2.isEmpty());

    item1.clear();
    item2.clear();

    grid.d->mode = CanvasGrid::Mode::Align;
    grid.setItems(in);
    EXPECT_EQ(item2, in);
    EXPECT_TRUE(item1.isEmpty());
}

TEST(CanvasGrid, items)
{
    CanvasGrid grid;
    grid.d->surfaces.insert(1, QSize(2,2));
    grid.d->surfaces.insert(2, QSize(2,3));
    grid.d->itemPos[1].insert("0,0", QPoint(0,0));
    grid.d->itemPos[1].insert("0,1", QPoint(0,1));
    grid.d->itemPos[1].insert("1,0", QPoint(1,0));
    grid.d->itemPos[2].insert("20,0", QPoint(0,0));

    {
        auto items = grid.items(1);
        ASSERT_EQ(items.size(), 3);
        EXPECT_EQ(items[0], QString("0,0"));
        EXPECT_EQ(items[1], QString("0,1"));
        EXPECT_EQ(items[2], QString("1,0"));
    }

    {
        auto items = grid.items(2);
        ASSERT_EQ(items.size(), 1);
        EXPECT_EQ(items[0], QString("20,0"));
    }

    {
        auto items = grid.items(3);
        EXPECT_EQ(items.size(), 0);
    }

    {
        auto items = grid.items();
        ASSERT_EQ(items.size(), 4);
        EXPECT_EQ(items[0], QString("0,0"));
        EXPECT_EQ(items[1], QString("0,1"));
        EXPECT_EQ(items[2], QString("1,0"));
        EXPECT_EQ(items[3], QString("20,0"));
    }
}

TEST(CanvasGrid, point)
{
    CanvasGrid grid;
    grid.d->itemPos[1].insert("0,0", QPoint(0,0));
    grid.d->itemPos[2].insert("20,0", QPoint(20,0));

    GridPos pos;
    EXPECT_FALSE(grid.point("1,1", pos));

    EXPECT_TRUE(grid.point("0,0", pos));
    EXPECT_EQ(pos.first, 1);
    EXPECT_EQ(pos.second, QPoint(0,0));

    EXPECT_TRUE(grid.point("20,0", pos));
    EXPECT_EQ(pos.first, 2);
    EXPECT_EQ(pos.second, QPoint(20,0));
}

TEST(CanvasGrid, overloadItems)
{
    CanvasGrid grid;
    grid.d->surfaces.insert(1, QSize(2,2));
    grid.d->surfaces.insert(2, QSize(2,3));
    grid.d->overload.append(QString("1"));

    EXPECT_TRUE(grid.overloadItems(0).isEmpty());
    EXPECT_TRUE(grid.overloadItems(1).isEmpty());
    EXPECT_TRUE(grid.overloadItems(3).isEmpty());

    auto item = grid.overloadItems(2);
    ASSERT_EQ(item.size(), 1);
    EXPECT_EQ(item.first(), QString("1"));
}

class TestCanvasGrid : public testing::Test
{
public:
    void SetUp() override {
        stub.set_lamda(&DisplayConfig::path, [this](){
            return QString("/tmp/dde-desktop-ut.conf");
        });

        conf = new DisplayConfig();
        stub.set_lamda(&DisplayConfig::instance, [this](){
            return conf;
        });

        stub.set_lamda(&CanvasGrid::requestSync, [this](){
           sync = true;
        });
    }

    void TearDown() {
        delete conf;
    }

    CanvasGrid grid;
    DisplayConfig *conf;
    stub_ext::StubExt stub;
    bool sync = false;
};

TEST_F(TestCanvasGrid, drop)
{
    grid.d->surfaces.insert(1, QSize(2,2));
    EXPECT_FALSE(grid.drop(1, QPoint(0,0), ""));
    EXPECT_FALSE(sync);

    sync = false;
    EXPECT_TRUE(grid.drop(1, QPoint(0,0), "0"));
    EXPECT_TRUE(sync);
    EXPECT_TRUE(grid.d->itemPos[1].contains("0"));
    ASSERT_EQ(grid.d->posItem[1].value(QPoint(0,0)), QString("0"));

    sync = false;
    EXPECT_FALSE(grid.drop(2, QPoint(0,0), "0"));
    EXPECT_FALSE(sync);

    sync = false;
    EXPECT_FALSE(grid.drop(1, QPoint(0,0), "1"));
    EXPECT_FALSE(sync);
    EXPECT_TRUE(grid.d->itemPos[1].contains("0"));
    EXPECT_FALSE(grid.d->itemPos[1].contains("1"));
    ASSERT_EQ(grid.d->posItem[1].value(QPoint(0,0)), QString("0"));
}

TEST_F(TestCanvasGrid, profiles)
{
    QStringList profile{QString("Screen_1"), QString("Screen_2")};
    stub.set_lamda(&DisplayConfig::profile, [&profile](){
        return profile;
    });

    QHash<QString, QHash<QString, QPoint>> coord;
    stub.set_lamda(&DisplayConfig::coordinates, [&coord](DisplayConfig *,
                   const QString &key){
        return coord.value(key);
    });
    coord["Screen_1"].insert("0",QPoint(0,0));
    coord["Screen_2"].insert("1",QPoint(0,1));
    coord["SingleScreen"].insert("2",QPoint(0,2));

    grid.d->surfaces.insert(1, QSize(0,0));
    {
        auto ret = grid.d->profiles();
        EXPECT_EQ(ret.size(), 1);
        ASSERT_TRUE(ret.contains(1));
        EXPECT_EQ(ret[1].size(), 1);
        EXPECT_EQ(ret[1].value("2"), QPoint(0,2));
    }

    grid.d->surfaces.insert(2, QSize(0,0));
    {
        auto ret = grid.d->profiles();
        EXPECT_EQ(ret.size(), 2);

        ASSERT_TRUE(ret.contains(1));
        ASSERT_TRUE(ret.contains(2));

        EXPECT_EQ(ret[1].size(), 1);
        EXPECT_EQ(ret[2].size(), 1);

        EXPECT_EQ(ret[1].value("0"), QPoint(0,0));
        EXPECT_EQ(ret[2].value("1"), QPoint(0,1));
    }
}

TEST_F(TestCanvasGrid, sequence)
{
    CanvasGrid grid;
    grid.d->surfaces.insert(1, QSize(1,2));
    grid.d->surfaces.insert(2, QSize(2,1));
    QStringList items;
    for (int i = 0; i < 5; ++i)
        items.append(QString::number(i));

    grid.d->sequence(items);

    EXPECT_EQ(grid.d->posItem[1].value(QPoint(0,0)), QString("0"));
    EXPECT_EQ(grid.d->posItem[1].value(QPoint(0,1)), QString("1"));
    EXPECT_EQ(grid.d->posItem[2].value(QPoint(0,0)), QString("2"));
    EXPECT_EQ(grid.d->posItem[2].value(QPoint(1,0)), QString("3"));
    ASSERT_EQ(grid.d->overload.size(), 1);
    EXPECT_EQ(grid.d->overload.first(), QString("4"));
}

TEST(CanvasGridSpecialist, profileIndex)
{
    EXPECT_EQ(CanvasGridSpecialist::profileIndex("SingleScreen"), 1);
    EXPECT_EQ(CanvasGridSpecialist::profileIndex("Screen_1"), 1);
    EXPECT_EQ(CanvasGridSpecialist::profileIndex("Screen_2"), 2);
}

TEST(CanvasGridSpecialist, profileKey)
{
    EXPECT_EQ(CanvasGridSpecialist::profileKey(1), QString("Screen_1"));
    EXPECT_EQ(CanvasGridSpecialist::profileKey(2), QString("Screen_2"));
}
