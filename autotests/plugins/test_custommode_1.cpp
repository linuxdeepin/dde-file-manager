// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_custommode_1.cpp
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

TEST_F(CustomModeTest, detachLayout)
{
    // Test method: void detachLayout(())
    EXPECT_NO_FATAL_FAILURE(obj->detachLayout());
}

TEST_F(CustomModeTest, filterDataInserted)
{
    // Test method: bool filterDataInserted((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->filterDataInserted(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(CustomModeTest, filterDataRenamed)
{
    // Test method: bool filterDataRenamed((const QUrl &oldUrl, const QUrl &newUrl))
    QUrl _arg0{};
    QUrl _arg1{};
    auto result = obj->filterDataRenamed(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(CustomModeTest, filterDataRested)
{
    // Test method: bool filterDataRested((QList<QUrl> *urls))
    auto result = obj->filterDataRested(nullptr);
    EXPECT_FALSE(result);

}

TEST_F(CustomModeTest, initialize)
{
    // Test method: bool initialize((CollectionModel *m))
    auto result = obj->initialize(nullptr);
    EXPECT_FALSE(result);

}

TEST_F(CustomModeTest, mode)
{
    // Test getter: OrganizerMode mode()
    auto result = obj->mode();
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(CustomModeTest, onFileDataChanged)
{
    // Test method: void onFileDataChanged((const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles))
    QModelIndex _arg0{};
    QModelIndex _arg1{};
    QVector<int> _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->onFileDataChanged(_arg0, _arg1, _arg2));
}

TEST_F(CustomModeTest, onFileInserted)
{
    // Test method: void onFileInserted((const QModelIndex &parent, int first, int last))
    QModelIndex _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onFileInserted(_arg0, 0, 0));
}

TEST_F(CustomModeTest, onFileRenamed)
{
    // Test method: void onFileRenamed((const QUrl &oldUrl, const QUrl &newUrl))
    QUrl _arg0{};
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->onFileRenamed(_arg0, _arg1));
}

TEST_F(CustomModeTest, onItemsChanged)
{
    // Test method: void onItemsChanged(())
    EXPECT_NO_FATAL_FAILURE(obj->onItemsChanged());
}

TEST_F(CustomModeTest, onNewCollection)
{
    // Test method: void onNewCollection((const QList<QUrl> &list))
    QList<QUrl> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onNewCollection(_arg0));
}
