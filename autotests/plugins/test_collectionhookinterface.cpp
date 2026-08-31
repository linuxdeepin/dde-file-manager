// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_collectionhookinterface.cpp
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

TEST_F(CollectionHookInterfaceTest, drawFile)
{
    // Test method: bool drawFile((const QString &viewId, const QUrl &file, QPainter *painter, const QStyleOptionViewItem *option, void *extData))
    QString _arg0{};
    QUrl _arg1{};
    auto result = obj->drawFile(_arg0, _arg1, nullptr, nullptr, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(CollectionHookInterfaceTest, dropData)
{
    // Test method: bool dropData((const QString &viewId, const QMimeData *md, const QPoint &viewPoint, void *extData))
    QString _arg0{};
    QPoint _arg2{};
    auto result = obj->dropData(_arg0, nullptr, _arg2, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(CollectionHookInterfaceTest, keyPress)
{
    // Test method: bool keyPress((const QString &viewId, int key, int modifiers, void *extData))
    QString _arg0{};
    auto result = obj->keyPress(_arg0, 0, 0, nullptr);
    EXPECT_FALSE(result);

}
