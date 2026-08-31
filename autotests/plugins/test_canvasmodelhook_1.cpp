// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_canvasmodelhook_1.cpp
 * @brief Unit tests for CanvasModelHook methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "hook/canvasmodelhook.h"

#include <QTest>

using namespace ddplugin_canvas;

class CanvasModelHookTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CanvasModelHook();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CanvasModelHook *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CanvasModelHookTest, dataChanged)
{
    // Test method: bool dataChanged((const QUrl &url, const QVector<int> &roles, void *extData))
    QUrl _arg0{};
    QVector<int> _arg1{};
    auto result = obj->dataChanged(_arg0, _arg1, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(CanvasModelHookTest, dataInserted)
{
    // Test method: bool dataInserted((const QUrl &url, void *extData))
    QUrl _arg0{};
    auto result = obj->dataInserted(_arg0, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(CanvasModelHookTest, dataRenamed)
{
    // Test method: bool dataRenamed((const QUrl &oldUrl, const QUrl &newUrl, void *extData))
    QUrl _arg0{};
    QUrl _arg1{};
    auto result = obj->dataRenamed(_arg0, _arg1, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(CanvasModelHookTest, dataRested)
{
    // Test method: bool dataRested((QList<QUrl> *urls, void *extData))
    auto result = obj->dataRested(nullptr, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(CanvasModelHookTest, dropMimeData)
{
    // Test method: bool dropMimeData((const QMimeData *data, const QUrl &dir, Qt::DropAction action, void *extData))
    QUrl _arg1{};
    auto result = obj->dropMimeData(nullptr, _arg1, Qt::DropAction(), nullptr);
    EXPECT_FALSE(result);

}

TEST_F(CanvasModelHookTest, hiddenFlagChanged)
{
    // Test method: void hiddenFlagChanged((bool show))
    EXPECT_NO_FATAL_FAILURE(obj->hiddenFlagChanged(false));
}

TEST_F(CanvasModelHookTest, mimeData)
{
    // Test method: bool mimeData((const QList<QUrl> &urls, QMimeData *out, void *extData))
    QList<QUrl> _arg0{};
    auto result = obj->mimeData(_arg0, nullptr, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(CanvasModelHookTest, mimeTypes)
{
    // Test method: bool mimeTypes((QStringList *types, void *extData))
    auto result = obj->mimeTypes(nullptr, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(CanvasModelHookTest, modelData)
{
    // Test method: bool modelData((const QUrl &url, int role, QVariant *out, void *extData))
    QUrl _arg0{};
    auto result = obj->modelData(_arg0, 0, nullptr, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(CanvasModelHookTest, sortData)
{
    // Test method: bool sortData((int role, int order, QList<QUrl> *files, void *extData))
    auto result = obj->sortData(0, 0, nullptr, nullptr);
    EXPECT_FALSE(result);

}
