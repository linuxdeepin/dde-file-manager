// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_modelhookinterface.cpp
 * @brief Unit tests for ModelHookInterface methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "model/modelhookinterface.h"

#include <QTest>

using namespace ddplugin_canvas;

class ModelHookInterfaceTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ModelHookInterface();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ModelHookInterface *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ModelHookInterfaceTest, dataRemoved)
{
    // Test method: bool dataRemoved((const QUrl &url, void *extData))
    QUrl _arg0{};
    auto result = obj->dataRemoved(_arg0, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(ModelHookInterfaceTest, mimeData)
{
    // Test method: bool mimeData((const QList<QUrl> &urls, QMimeData *out, void *extData))
    QList<QUrl> _arg0{};
    auto result = obj->mimeData(_arg0, nullptr, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(ModelHookInterfaceTest, mimeTypes)
{
    // Test method: bool mimeTypes((QStringList *types, void *extData))
    auto result = obj->mimeTypes(nullptr, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(ModelHookInterfaceTest, modelData)
{
    // Test method: bool modelData((const QUrl &url, int role, QVariant *out, void *extData))
    QUrl _arg0{};
    auto result = obj->modelData(_arg0, 0, nullptr, nullptr);
    EXPECT_FALSE(result);

}
