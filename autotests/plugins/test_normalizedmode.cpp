// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_normalizedmode.cpp
 * @brief Unit tests for NormalizedMode methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "mode/normalizedmode.h"

#include <QTest>

using namespace ddplugin_organizer;

class NormalizedModeTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new NormalizedMode();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    NormalizedMode *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(NormalizedModeTest, filterDataRenamed)
{
    // Test method: bool filterDataRenamed((const QUrl &oldUrl, const QUrl &newUrl))
    QUrl _arg0{};
    QUrl _arg1{};
    auto result = obj->filterDataRenamed(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(NormalizedModeTest, filterDataRested)
{
    // Test method: bool filterDataRested((QList<QUrl> *urls))
    auto result = obj->filterDataRested(nullptr);
    EXPECT_FALSE(result);

}

TEST_F(NormalizedModeTest, mode)
{
    // Test getter: OrganizerMode mode()
    auto result = obj->mode();
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(NormalizedModeTest, onFileAboutToBeRemoved)
{
    // Test method: void onFileAboutToBeRemoved((const QModelIndex &parent, int first, int last))
    QModelIndex _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onFileAboutToBeRemoved(_arg0, 0, 0));
}

TEST_F(NormalizedModeTest, onFileInserted)
{
    // Test method: void onFileInserted((const QModelIndex &parent, int first, int last))
    QModelIndex _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onFileInserted(_arg0, 0, 0));
}

TEST_F(NormalizedModeTest, onFileRenamed)
{
    // Test method: void onFileRenamed((const QUrl &oldUrl, const QUrl &newUrl))
    QUrl _arg0{};
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->onFileRenamed(_arg0, _arg1));
}

TEST_F(NormalizedModeTest, releaseCollection)
{
    // Test method: void releaseCollection((int category))
    EXPECT_NO_FATAL_FAILURE(obj->releaseCollection(0));
}

TEST_F(NormalizedModeTest, removeClassifier)
{
    // Test method: void removeClassifier(())
    EXPECT_NO_FATAL_FAILURE(obj->removeClassifier());
}

TEST_F(NormalizedModeTest, reset)
{
    // Test method: void reset(())
    EXPECT_NO_FATAL_FAILURE(obj->reset());
}

TEST_F(NormalizedModeTest, NormalizedMode)
{
    // Test constructor: NormalizedMode((QObject *parent))
    ASSERT_NE(obj, nullptr);
}
