// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include "stubext.h"

#include "plugins/desktop/ddplugin-canvas/grid/canvasgrid.h"
#include "plugins/desktop/ddplugin-canvas/grid/gridcore.h"

#include <QPoint>
#include <QSize>
#include <QStringList>
#include <QTimer>

using namespace ddplugin_canvas;

class CanvasGridImpl : public testing::Test
{
protected:
    void SetUp() override
    {
        grid = CanvasGrid::instance();

        // Prevent delayed sync from touching DisplayConfig during tests.
        using VoidStart = void (QTimer::*)();
        using IntStart = void (QTimer::*)(int);
        stub.set_lamda(static_cast<VoidStart>(&QTimer::start), [](QTimer *) {});
        stub.set_lamda(static_cast<IntStart>(&QTimer::start), [](QTimer *, int) {});

        grid->requestSync(0);
        grid->initSurface(0);
        grid->setMode(CanvasGrid::Mode::Custom);
    }

    void TearDown() override
    {
        grid->initSurface(0);
        grid->setMode(CanvasGrid::Mode::Custom);
        stub.clear();
    }

    CanvasGrid *grid = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CanvasGridImpl, instanceAndSurface)
{
    EXPECT_NE(grid, nullptr);
    EXPECT_EQ(grid, CanvasGrid::instance());

    grid->initSurface(1);
    grid->updateSize(1, QSize(2, 3));
    EXPECT_EQ(grid->surfaceSize(1), QSize(2, 3));
    EXPECT_EQ(grid->gridCount(1), 6);
    EXPECT_EQ(grid->gridCount(), 6);

    // Same size is ignored.
    grid->updateSize(1, QSize(2, 3));
    EXPECT_EQ(grid->surfaceSize(1), QSize(2, 3));
}

TEST_F(CanvasGridImpl, modeAndItems)
{
    grid->initSurface(1);
    grid->updateSize(1, QSize(2, 2));

    grid->setMode(CanvasGrid::Mode::Align);
    EXPECT_EQ(grid->mode(), CanvasGrid::Mode::Align);

    grid->setItems({ "c", "a", "b" });
    QStringList items = grid->items(1);
    EXPECT_EQ(items.size(), 3);

    EXPECT_EQ(grid->item(1, QPoint(0, 0)), QString("c"));
    EXPECT_EQ(grid->item(1, QPoint(0, 1)), QString("a"));

    QHash<QString, QPoint> pts = grid->points(1);
    EXPECT_TRUE(pts.contains("a"));

    QPair<int, QPoint> pos;
    EXPECT_TRUE(grid->point("a", pos));
    EXPECT_EQ(pos.first, 1);
    EXPECT_EQ(pos.second, QPoint(0, 1));
    EXPECT_FALSE(grid->point(QString(), pos));
}

TEST_F(CanvasGridImpl, overloadAndPop)
{
    grid->initSurface(1);
    grid->updateSize(1, QSize(1, 1));

    grid->setItems({ "a", "b" });
    // items() includes overload items on the last screen (see CanvasGrid::items).
    EXPECT_EQ(grid->items(1).size(), 2);
    EXPECT_EQ(grid->overloadItems(1).size(), 1);
    EXPECT_EQ(grid->overloadItems(1).first(), QString("b"));

    // Create a free slot and pop the overload item into it.
    grid->initSurface(1);
    grid->updateSize(1, QSize(2, 1));
    grid->setItems({ "a", "b" });
    EXPECT_EQ(grid->overloadItems(1).size(), 0);
    grid->append("c");
    EXPECT_EQ(grid->overloadItems(1).size(), 1);
    grid->remove(1, "a");
    grid->popOverload();
    EXPECT_EQ(grid->overloadItems(1).size(), 0);
    EXPECT_EQ(grid->item(1, QPoint(0, 0)), QString("c"));
}

TEST_F(CanvasGridImpl, dropMoveRemoveReplace)
{
    grid->initSurface(1);
    grid->updateSize(1, QSize(2, 2));
    grid->setItems({ "a" });

    EXPECT_TRUE(grid->drop(1, QPoint(1, 1), "b"));
    EXPECT_EQ(grid->item(1, QPoint(1, 1)), QString("b"));
    EXPECT_FALSE(grid->drop(1, QPoint(1, 1), "c")); // occupied

    EXPECT_TRUE(grid->move(1, QPoint(0, 1), "b", { "b" }));
    EXPECT_EQ(grid->item(1, QPoint(0, 1)), QString("b"));

    EXPECT_TRUE(grid->remove(1, "b"));
    EXPECT_EQ(grid->item(1, QPoint(0, 1)), QString());
    EXPECT_FALSE(grid->remove(1, "b"));

    EXPECT_TRUE(grid->drop(1, QPoint(1, 0), "old"));
    EXPECT_TRUE(grid->replace("old", "new"));
    EXPECT_EQ(grid->item(1, QPoint(1, 0)), QString("new"));
    EXPECT_FALSE(grid->replace("missing", "x"));
}

TEST_F(CanvasGridImpl, appendAndTryAppendAfter)
{
    grid->initSurface(1);
    grid->updateSize(1, QSize(3, 1));

    grid->append("a");
    EXPECT_EQ(grid->items(1).size(), 1);

    grid->append({ "b", "c" });
    EXPECT_EQ(grid->items(1).size(), 3);

    grid->tryAppendAfter({ "d", "e" }, 1, QPoint(0, 0));
    EXPECT_EQ(grid->items(1).size(), 5);
    EXPECT_TRUE(grid->items(1).contains("d"));
    EXPECT_TRUE(grid->items(1).contains("e"));

    // Empty inputs are ignored.
    grid->append(QString());
    grid->append(QStringList());
    grid->tryAppendAfter(QStringList(), 1, QPoint(0, 0));
}

TEST_F(CanvasGridImpl, arrangeAndRequestSync)
{
    grid->initSurface(1);
    grid->updateSize(1, QSize(2, 2));
    grid->setMode(CanvasGrid::Mode::Align);
    grid->setItems({ "z", "y" });

    EXPECT_NO_THROW(grid->arrange());
    EXPECT_EQ(grid->items(1).size(), 2);

    EXPECT_NO_THROW(grid->requestSync());
    EXPECT_NO_THROW(grid->requestSync(50));
}

TEST_F(CanvasGridImpl, coreOperations)
{
    grid->initSurface(1);
    grid->updateSize(1, QSize(2, 2));
    grid->setItems({ "a" });

    GridCore &core = grid->core();
    EXPECT_EQ(core.surfaceSize(1), QSize(2, 2));

    core.insert(1, QPoint(1, 0), "b");
    EXPECT_EQ(grid->item(1, QPoint(1, 0)), QString("b"));

    QList<QPoint> voids = core.voidPos(1);
    EXPECT_EQ(voids.size(), 2);

    GridPos pos;
    EXPECT_TRUE(core.position("b", pos));
    EXPECT_EQ(pos, GridPos(1, QPoint(1, 0)));
    EXPECT_EQ(core.item(pos), QString("b"));

    EXPECT_FALSE(core.isFull(1));

    core.removeAll({ "a", "b" });
    EXPECT_TRUE(grid->items(1).isEmpty());
}
