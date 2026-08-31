// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_canvasgridshell.cpp
 * @brief Unit tests for CanvasGridShell methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "interface/canvasgridshell.h"

#include <QTest>

using namespace ddplugin_organizer;

class CanvasGridShellTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CanvasGridShell();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CanvasGridShell *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CanvasGridShellTest, item)
{
    // Test method: QString item((int index, const QPoint &gridPos))
    QPoint _arg1{};
    auto result = obj->item(0, _arg1);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(CanvasGridShellTest, point)
{
    // Test method: int point((const QString &item, QPoint *pos))
    QString _arg0{};
    auto result = obj->point(_arg0, nullptr);
    EXPECT_GE(result, 0);

}
