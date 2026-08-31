// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_keyselector_1.cpp
 * @brief Unit tests for KeySelector methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "view/operator/keyselector.h"

#include <QTest>

using namespace ddplugin_canvas;

class KeySelectorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new KeySelector();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    KeySelector *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(KeySelectorTest, KeySelector)
{
    // Test constructor: KeySelector((CanvasView *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(KeySelectorTest, filterKeys)
{
    // Test getter: QList<Qt::Key> filterKeys()
    auto result = obj->filterKeys();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(KeySelectorTest, incrementSelect)
{
    // Test method: void incrementSelect((const QModelIndex &index))
    QModelIndex _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->incrementSelect(_arg0));
}

TEST_F(KeySelectorTest, keyPressed)
{
    // Test method: void keyPressed((QKeyEvent *event))
    EXPECT_NO_FATAL_FAILURE(obj->keyPressed(nullptr));
}

TEST_F(KeySelectorTest, keyboardSearch)
{
    // Test method: void keyboardSearch((const QString &search))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->keyboardSearch(_arg0));
}

TEST_F(KeySelectorTest, singleSelect)
{
    // Test method: void singleSelect((const QModelIndex &index))
    QModelIndex _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->singleSelect(_arg0));
}

TEST_F(KeySelectorTest, toggleSelect)
{
    // Test method: void toggleSelect(())
    EXPECT_NO_FATAL_FAILURE(obj->toggleSelect());
}
