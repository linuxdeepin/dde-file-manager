// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_canvasgridbroker_1.cpp
 * @brief Unit tests for CanvasGridBroker methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "broker/canvasgridbroker.h"

#include <QTest>

using namespace ddplugin_canvas;

class CanvasGridBrokerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CanvasGridBroker();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CanvasGridBroker *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CanvasGridBrokerTest, CanvasGridBroker)
{
    // Test constructor: CanvasGridBroker((CanvasGrid *gridPtr, QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(CanvasGridBrokerTest, init)
{
    // Test bool getter: init()
    bool result = obj->init();
    EXPECT_FALSE(result);

}

TEST_F(CanvasGridBrokerTest, point)
{
    // Test method: int point((const QString &item, QPoint *pos))
    QString _arg0{};
    auto result = obj->point(_arg0, nullptr);
    EXPECT_GE(result, 0);

}

TEST_F(CanvasGridBrokerTest, tryAppendAfter)
{
    // Test method: void tryAppendAfter((const QStringList &items, int index, const QPoint &begin))
    QStringList _arg0{};
    QPoint _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->tryAppendAfter(_arg0, 0, _arg2));
}

TEST_F(CanvasGridBrokerTest, CanvasGridBroker_Destructor)
{
    // Test method:  ~CanvasGridBroker(())
    EXPECT_NO_FATAL_FAILURE({ CanvasGridBroker *tmp = new CanvasGridBroker(); delete tmp; });
}
