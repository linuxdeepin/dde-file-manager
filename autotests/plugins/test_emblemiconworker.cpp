// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_emblemiconworker.cpp
 * @brief Unit tests for EmblemIconWorker methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "extensionimpl/emblemimpl/extensionemblemmanager.h"

#include <QTest>

using namespace dfmplugin_utils;

class EmblemIconWorkerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new EmblemIconWorker();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    EmblemIconWorker *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(EmblemIconWorkerTest, hasCachedByOtherLocationEmblem)
{
    // Test method: bool hasCachedByOtherLocationEmblem((const QString &path, quint64 addr))
    QString _arg0{};
    auto result = obj->hasCachedByOtherLocationEmblem(_arg0, 0);
    EXPECT_FALSE(result);

}

TEST_F(EmblemIconWorkerTest, onClearCache)
{
    // Test method: void onClearCache(())
    EXPECT_NO_FATAL_FAILURE(obj->onClearCache());
}

TEST_F(EmblemIconWorkerTest, saveToPluginCache)
{
    // Test method: void saveToPluginCache((quint64 addr, const QString &path, const QList<QPair<QString, int>> &group))
    QString _arg1{};
    QList<QPair<QString, int>> _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->saveToPluginCache(0, _arg1, _arg2));
}

TEST_F(EmblemIconWorkerTest, updateLayoutGroup)
{
    // Test method: QList<QPair<QString, int>> updateLayoutGroup((const QList<QPair<QString, int>> &cache, const QList<QPair<QString, int>> &group))
    QList<QPair<QString, int>> _arg0{};
    QList<QPair<QString, int>> _arg1{};
    auto result = obj->updateLayoutGroup(_arg0, _arg1);
    EXPECT_TRUE(result.isEmpty());

}
