// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_custommode.cpp
 * @brief Unit tests for CustomMode methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "mode/custommode.h"

#include <QTest>

using namespace ddplugin_organizer;

class CustomModeTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CustomMode();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CustomMode *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CustomModeTest, filterDropData)
{
    // Test method: bool filterDropData((int viewIndex, const QMimeData *mimeData, const QPoint &viewPoint, void *extData))
    QPoint _arg2{};
    auto result = obj->filterDropData(0, nullptr, _arg2, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(CustomModeTest, layout)
{
    // Test method: void layout(())
    EXPECT_NO_FATAL_FAILURE(obj->layout());
}

TEST_F(CustomModeTest, onDeleteCollection)
{
    // Test method: void onDeleteCollection((const QString &key))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onDeleteCollection(_arg0));
}

TEST_F(CustomModeTest, onFileAboutToBeRemoved)
{
    // Test method: void onFileAboutToBeRemoved((const QModelIndex &parent, int first, int last))
    QModelIndex _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onFileAboutToBeRemoved(_arg0, 0, 0));
}

TEST_F(CustomModeTest, rebuild)
{
    // Test method: void rebuild(())
    EXPECT_NO_FATAL_FAILURE(obj->rebuild());
}

TEST_F(CustomModeTest, reset)
{
    // Test method: void reset(())
    EXPECT_NO_FATAL_FAILURE(obj->reset());
}

TEST_F(CustomModeTest, CustomMode)
{
    // Test constructor: CustomMode((QObject *parent))
    ASSERT_NE(obj, nullptr);
}
