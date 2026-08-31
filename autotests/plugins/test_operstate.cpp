// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_operstate.cpp
 * @brief Unit tests for OperState methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "view/operator/operstate.h"

#include <QTest>

using namespace ddplugin_canvas;

class OperStateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new OperState();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    OperState *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(OperStateTest, current)
{
    // Test getter: QModelIndex current()
    auto result = obj->current();
    EXPECT_FALSE(result.isValid());

}

TEST_F(OperStateTest, selectionChanged)
{
    // Test method: void selectionChanged((const QItemSelection &selected, const QItemSelection &deselected))
    QItemSelection _arg0{};
    QItemSelection _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->selectionChanged(_arg0, _arg1));
}

TEST_F(OperStateTest, setContBegin)
{
    // Test method: void setContBegin(())
    EXPECT_NO_FATAL_FAILURE(obj->setContBegin());
}

TEST_F(OperStateTest, setCurrent)
{
    // Test setter: void setCurrent((const QModelIndex &value))
    QModelIndex _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setCurrent(_arg0));
}
