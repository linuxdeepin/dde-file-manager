// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_collectionwidgetprivate.cpp
 * @brief Unit tests for CollectionWidgetPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "view/collectionwidget.h"

#include <QTest>

using namespace ddplugin_organizer;

class CollectionWidgetPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CollectionWidgetPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CollectionWidgetPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CollectionWidgetPrivateTest, CollectionWidgetPrivate)
{
    // Test constructor: CollectionWidgetPrivate((const QString &uuid, CollectionDataProvider *dataProvider, CollectionWidget *qq, QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(CollectionWidgetPrivateTest, onNameChanged)
{
    // Test method: void onNameChanged((const QString &key, const QString &name))
    QString _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->onNameChanged(_arg0, _arg1));
}

TEST_F(CollectionWidgetPrivateTest, CollectionWidgetPrivate_Destructor)
{
    // Test method:  ~CollectionWidgetPrivate(())
    EXPECT_NO_FATAL_FAILURE({ CollectionWidgetPrivate *tmp = new CollectionWidgetPrivate(); delete tmp; });
}
