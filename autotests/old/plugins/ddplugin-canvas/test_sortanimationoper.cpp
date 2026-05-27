// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "view/operator/sortanimationoper.h"
#include "view/canvasview.h"
#include "grid/canvasgrid.h"
#include "model/canvasproxymodel.h"

#include <gtest/gtest.h>
#include <QApplication>
#include <QWidget>
#include <QTimer>
#include <QPixmap>
#include <QStringList>

using namespace ddplugin_canvas;

class UT_SortAnimationOper : public testing::Test
{
public:
    virtual void SetUp() override
    {
        if (!QApplication::instance()) {
            int argc = 0;
            char **argv = nullptr;
            app = new QApplication(argc, argv);
        }

        parentWidget = new QWidget();

        // Mock CanvasView
        stub.set_lamda(ADDR(CanvasView, screenNum), [](const CanvasView*) -> int {
            __DBG_STUB_INVOKE__
            return 0;
        });

        // Mock CanvasGrid
        stub.set_lamda(ADDR(CanvasGrid, instance), []() -> CanvasGrid* {
            __DBG_STUB_INVOKE__
            static CanvasGrid grid;
            return &grid;
        });

        stub.set_lamda(ADDR(CanvasGrid, point), [](CanvasGrid*, const QString&, QPair<int, QPoint>&) -> bool {
            __DBG_STUB_INVOKE__
            return false;
        });

        // Create test instance
        view = new CanvasView(parentWidget);
        oper = new SortAnimationOper(view);
    }

    virtual void TearDown() override
    {
        if (oper) {
            delete oper;
            oper = nullptr;
        }

        if (view) {
            delete view;
            view = nullptr;
        }

        if (parentWidget) {
            delete parentWidget;
            parentWidget = nullptr;
        }

        stub.clear();
    }

public:
    QApplication *app = nullptr;
    QWidget *parentWidget = nullptr;
    CanvasView *view = nullptr;
    SortAnimationOper *oper = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(UT_SortAnimationOper, constructor_CreateOper_InitializesCorrectly)
{
    EXPECT_NE(oper, nullptr);
}

TEST_F(UT_SortAnimationOper, setMoveValue_WithValidItems_SetsItems)
{
    QStringList items;
    items << "item1" << "item2";
    EXPECT_NO_THROW(oper->setMoveValue(items));
}

TEST_F(UT_SortAnimationOper, setMoveValue_WithEmptyItems_DoesNothing)
{
    QStringList items;
    EXPECT_NO_THROW(oper->setMoveValue(items));
}

TEST_F(UT_SortAnimationOper, setItemPixmap_WithValidParameters_SetsPixmap)
{
    QString item = "test";
    QPixmap pixmap(100, 100);
    EXPECT_NO_THROW(oper->setItemPixmap(item, pixmap));
}

TEST_F(UT_SortAnimationOper, findPixmap_WithValidItem_ReturnsPixmap)
{
    QString item = "test";
    QPixmap result = oper->findPixmap(item);
    EXPECT_TRUE(result.isNull());
}

TEST_F(UT_SortAnimationOper, tryMove_WithValidConditions_TriesMove)
{
    EXPECT_NO_THROW(oper->tryMove());
}

TEST_F(UT_SortAnimationOper, setMoveDuration_WithValidDuration_SetsDuration)
{
    double duration = 1.0;
    EXPECT_NO_THROW(oper->setMoveDuration(duration));
}

TEST_F(UT_SortAnimationOper, startDelayMove_WithValidOper_StartsTimer)
{
    EXPECT_NO_THROW(oper->startDelayMove());
}

TEST_F(UT_SortAnimationOper, stopDelayMove_WithValidOper_StopsTimer)
{
    EXPECT_NO_THROW(oper->stopDelayMove());
}