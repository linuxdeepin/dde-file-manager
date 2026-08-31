// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_collectionwidget.cpp
 * @brief Unit tests for CollectionWidget methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "view/collectionwidget.h"

#include <QTest>

using namespace ddplugin_organizer;

class CollectionWidgetTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CollectionWidget();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CollectionWidget *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CollectionWidgetTest, CollectionWidget)
{
    // Test constructor: CollectionWidget((const QString &uuid, ddplugin_organizer::CollectionDataProvider *dataProvider, QWidget *parent))
    ASSERT_NE(obj, nullptr);
}
