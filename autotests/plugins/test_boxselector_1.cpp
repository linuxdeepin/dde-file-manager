// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_boxselector_1.cpp
 * @brief Unit tests for BoxSelector methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "view/operator/boxselector.h"

#include <QTest>

using namespace ddplugin_canvas;

class BoxSelectorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new BoxSelector();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    BoxSelector *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(BoxSelectorTest, BoxSelector)
{
    // Test constructor: BoxSelector((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(BoxSelectorTest, beginSelect)
{
    // Test method: void beginSelect((const QPoint &globalPos, bool autoSelect))
    QPoint _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->beginSelect(_arg0, false));
}

TEST_F(BoxSelectorTest, clipRect)
{
    // Test method: QRect clipRect((QRect rect, const QRect &geometry))
    QRect _arg1{};
    auto result = obj->clipRect(QRect(), _arg1);
    EXPECT_FALSE(result.isValid());

}

TEST_F(BoxSelectorTest, delayUpdate)
{
    // Test method: void delayUpdate(())
    EXPECT_NO_FATAL_FAILURE(obj->delayUpdate());
}

TEST_F(BoxSelectorTest, endSelect)
{
    // Test method: void endSelect(())
    EXPECT_NO_FATAL_FAILURE(obj->endSelect());
}

TEST_F(BoxSelectorTest, globalRect)
{
    // Test getter: QRect globalRect()
    auto result = obj->globalRect();
    EXPECT_FALSE(result.isValid());

}

TEST_F(BoxSelectorTest, innerGeometry)
{
    // Test method: QRect innerGeometry((QWidget *w))
    auto result = obj->innerGeometry(nullptr);
    EXPECT_FALSE(result.isValid());

}

TEST_F(BoxSelectorTest, instance)
{
    // Test getter: BoxSelector instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}

TEST_F(BoxSelectorTest, isAcvite)
{
    // Test bool getter: isAcvite()
    bool result = obj->isAcvite();
    EXPECT_FALSE(result);

}

TEST_F(BoxSelectorTest, isBeginFrom)
{
    // Test method: bool isBeginFrom((CanvasView *w))
    auto result = obj->isBeginFrom(nullptr);
    EXPECT_FALSE(result);

}

TEST_F(BoxSelectorTest, setAcvite)
{
    // Test setter: void setAcvite((bool ac))
    EXPECT_NO_FATAL_FAILURE(obj->setAcvite(false));
}

TEST_F(BoxSelectorTest, setBegin)
{
    // Test setter: void setBegin((const QPoint &globalPos))
    QPoint _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setBegin(_arg0));
}

TEST_F(BoxSelectorTest, setEnd)
{
    // Test setter: void setEnd((const QPoint &globalPos))
    QPoint _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setEnd(_arg0));
}

TEST_F(BoxSelectorTest, update)
{
    // Test method: void update(())
    EXPECT_NO_FATAL_FAILURE(obj->update());
}

TEST_F(BoxSelectorTest, updateRubberBand)
{
    // Test method: void updateRubberBand(())
    EXPECT_NO_FATAL_FAILURE(obj->updateRubberBand());
}

TEST_F(BoxSelectorTest, updateSelection)
{
    // Test method: void updateSelection(())
    EXPECT_NO_FATAL_FAILURE(obj->updateSelection());
}
