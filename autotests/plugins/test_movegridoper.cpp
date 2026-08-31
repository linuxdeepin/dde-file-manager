// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_movegridoper.cpp
 * @brief Unit tests for MoveGridOper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "grid/gridcore.h"

#include <QTest>

using namespace ddplugin_canvas;

class MoveGridOperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new MoveGridOper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    MoveGridOper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(MoveGridOperTest, calcDestination)
{
    // Test method: void calcDestination((const QStringList &orgItems, const GridPos &ref, const QPoint &focus,
                                   QHash<QString, QPoint> &dest, QStringList &invalid))
    QStringList _arg0{};
    GridPos _arg1{};
    QPoint _arg2{};
    QHash<QString, QPoint> _arg3{};
    QStringList _arg4{};
    EXPECT_NO_FATAL_FAILURE(obj->calcDestination(_arg0, _arg1, _arg2, _arg3, _arg4));
}

TEST_F(MoveGridOperTest, move)
{
    // Test method: bool move((const GridPos &to, const GridPos &center, const QStringList &moveItems))
    GridPos _arg0{};
    GridPos _arg1{};
    QStringList _arg2{};
    auto result = obj->move(_arg0, _arg1, _arg2);
    EXPECT_FALSE(result);

}
