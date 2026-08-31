// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_collectionhookinterface_1.cpp
 * @brief Unit tests for CollectionHookInterface methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "broker/collectionhookinterface.h"

#include <QTest>

using namespace ddplugin_organizer;

class CollectionHookInterfaceTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CollectionHookInterface();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CollectionHookInterface *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CollectionHookInterfaceTest, dragMove)
{
    // Test method: bool dragMove((const QString &viewId, const QMimeData *mime, const QPoint &viewPos, void *extData))
    QString _arg0{};
    QPoint _arg2{};
    auto result = obj->dragMove(_arg0, nullptr, _arg2, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(CollectionHookInterfaceTest, keyboardSearch)
{
    // Test method: bool keyboardSearch((const QString &viewId, const QString &search, void *extData))
    QString _arg0{};
    QString _arg1{};
    auto result = obj->keyboardSearch(_arg0, _arg1, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(CollectionHookInterfaceTest, shortcutkeyPress)
{
    // Test method: bool shortcutkeyPress((const QString &viewId, int key, int modifiers, void *extData))
    QString _arg0{};
    auto result = obj->shortcutkeyPress(_arg0, 0, 0, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(CollectionHookInterfaceTest, startDrag)
{
    // Test method: bool startDrag((const QString &viewId, int supportedActions, void *extData))
    QString _arg0{};
    auto result = obj->startDrag(_arg0, 0, nullptr);
    EXPECT_FALSE(result);

}
