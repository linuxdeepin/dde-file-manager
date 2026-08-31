// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_oemmenu_1.cpp
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

TEST_F(OemMenuTest, OemMenu)
{
    // Test constructor: OemMenu((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(OemMenuTest, emptyActions)
{
    // Test method: QList<QAction *> emptyActions((const QUrl &currentDir, bool onDesktop))
    QUrl _arg0{};
    auto result = obj->emptyActions(_arg0, false);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(OemMenuTest, OemMenu_Destructor)
{
    // Test method:  ~OemMenu(())
    EXPECT_NO_FATAL_FAILURE({ OemMenu *tmp = new OemMenu(); delete tmp; });
}
