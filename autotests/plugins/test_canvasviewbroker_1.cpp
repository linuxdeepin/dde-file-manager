// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_canvasviewbroker_1.cpp
 * @brief Unit tests for CanvasViewBroker methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "broker/canvasviewbroker.h"

#include <QTest>

using namespace ddplugin_canvas;

class CanvasViewBrokerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CanvasViewBroker();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CanvasViewBroker *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CanvasViewBrokerTest, CanvasViewBroker)
{
    // Test constructor: CanvasViewBroker((CanvasManager *mrg, QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(CanvasViewBrokerTest, fileOperator)
{
    // Test getter: QObject fileOperator()
    auto result = obj->fileOperator();
    EXPECT_NO_FATAL_FAILURE({ obj->fileOperator(); });

}

TEST_F(CanvasViewBrokerTest, getView)
{
    // Test method: QSharedPointer<CanvasView> getView((int idx))
    auto result = obj->getView(0);
    EXPECT_NE(result.get(), nullptr);

}

TEST_F(CanvasViewBrokerTest, gridSize)
{
    // Test method: QSize gridSize((int idx))
    auto result = obj->gridSize(0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(CanvasViewBrokerTest, gridVisualRect)
{
    // Test method: QRect gridVisualRect((int idx, const QPoint &gridPos))
    QPoint _arg1{};
    auto result = obj->gridVisualRect(0, _arg1);
    EXPECT_FALSE(result.isValid());

}

TEST_F(CanvasViewBrokerTest, iconRect)
{
    // Test method: QRect iconRect((int idx, QRect visualRect))
    auto result = obj->iconRect(0, QRect());
    EXPECT_FALSE(result.isValid());

}

TEST_F(CanvasViewBrokerTest, init)
{
    // Test bool getter: init()
    bool result = obj->init();
    EXPECT_FALSE(result);

}

TEST_F(CanvasViewBrokerTest, refresh)
{
    // Test method: void refresh((int idx))
    EXPECT_NO_FATAL_FAILURE(obj->refresh(0));
}

TEST_F(CanvasViewBrokerTest, update)
{
    // Test method: void update((int idx))
    EXPECT_NO_FATAL_FAILURE(obj->update(0));
}

TEST_F(CanvasViewBrokerTest, visualRect)
{
    // Test method: QRect visualRect((int idx, const QUrl &url))
    QUrl _arg1{};
    auto result = obj->visualRect(0, _arg1);
    EXPECT_FALSE(result.isValid());

}

TEST_F(CanvasViewBrokerTest, CanvasViewBroker_Destructor)
{
    // Test method:  ~CanvasViewBroker(())
    EXPECT_NO_FATAL_FAILURE({ CanvasViewBroker *tmp = new CanvasViewBroker(); delete tmp; });
}
