// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_extendmenusceneprivate.cpp
 * @brief Unit tests for ExtendMenuScenePrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "extendmenuscene/extendmenuscene.h"

#include <QTest>

using namespace dfmplugin_menu;

class ExtendMenuScenePrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ExtendMenuScenePrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ExtendMenuScenePrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ExtendMenuScenePrivateTest, childActions)
{
    // Test method: QList<QAction *> childActions((QAction *action))
    auto result = obj->childActions(nullptr);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(ExtendMenuScenePrivateTest, getSubMenus)
{
    // Test method: void getSubMenus((QMenu *currMenu, const QString &parentMenuName, QMap<QString, QMenu *> &subMenus))
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->getSubMenus(nullptr, _arg1, nullptr));
}

TEST_F(ExtendMenuScenePrivateTest, insertIntoExistedSubActions)
{
    // Test method: bool insertIntoExistedSubActions((QAction *action, QMap<QString, QList<QAction *>> &extSubActMap))
    auto result = obj->insertIntoExistedSubActions(nullptr, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(ExtendMenuScenePrivateTest, mergeSubActions)
{
    // Test method: void mergeSubActions((const QMap<QString, QList<QAction *>> &extSubActMap, const QMap<QString, QMenu *> &subMenus))
    EXPECT_NO_FATAL_FAILURE(obj->mergeSubActions(nullptr, nullptr));
}
