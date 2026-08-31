// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_oemmenu.cpp
 * @brief Unit tests for OemMenu methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "oemmenuscene/oemmenu.h"

#include <QTest>

using namespace dfmplugin_menu;

class OemMenuTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new OemMenu();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    OemMenu *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(OemMenuTest, makeCommand)
{
    // Test method: QPair<QString, QStringList> makeCommand((const QAction *action, const QUrl &dir, const QUrl &focus, const QList<QUrl> &files))
    QUrl _arg1{};
    QUrl _arg2{};
    QList<QUrl> _arg3{};
    auto result = obj->makeCommand(nullptr, _arg1, _arg2, _arg3);
    EXPECT_TRUE(result.isEmpty());

}
