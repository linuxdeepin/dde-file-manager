// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_modelhookinterface_1.cpp
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

TEST_F(ModelHookInterfaceTest, ModelHookInterface)
{
    // Test constructor: ModelHookInterface(())
    ASSERT_NE(obj, nullptr);
}

TEST_F(ModelHookInterfaceTest, dataChanged)
{
    // Test method: bool dataChanged((const QUrl &url, const QVector<int> &roles, void *extData))
    QUrl _arg0{};
    QVector<int> _arg1{};
    auto result = obj->dataChanged(_arg0, _arg1, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(ModelHookInterfaceTest, dataInserted)
{
    // Test method: bool dataInserted((const QUrl &url, void *extData))
    QUrl _arg0{};
    auto result = obj->dataInserted(_arg0, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(ModelHookInterfaceTest, dataRenamed)
{
    // Test method: bool dataRenamed((const QUrl &oldUrl, const QUrl &newUrl, void *extData))
    QUrl _arg0{};
    QUrl _arg1{};
    auto result = obj->dataRenamed(_arg0, _arg1, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(ModelHookInterfaceTest, dataRested)
{
    // Test method: bool dataRested((QList<QUrl> *urls, void *extData))
    auto result = obj->dataRested(nullptr, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(ModelHookInterfaceTest, dropMimeData)
{
    // Test method: bool dropMimeData((const QMimeData *data, const QUrl &dir, Qt::DropAction action, void *extData))
    QUrl _arg1{};
    auto result = obj->dropMimeData(nullptr, _arg1, Qt::DropAction(), nullptr);
    EXPECT_FALSE(result);

}

TEST_F(ModelHookInterfaceTest, hiddenFlagChanged)
{
    // Test method: void hiddenFlagChanged((bool show))
    EXPECT_NO_FATAL_FAILURE(obj->hiddenFlagChanged(false));
}

TEST_F(ModelHookInterfaceTest, sortData)
{
    // Test method: bool sortData((int role, int order, QList<QUrl> *files, void *extData))
    auto result = obj->sortData(0, 0, nullptr, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(ModelHookInterfaceTest, ModelHookInterface_Destructor)
{
    // Test method:  ~ModelHookInterface(())
    EXPECT_NO_FATAL_FAILURE({ ModelHookInterface *tmp = new ModelHookInterface(); delete tmp; });
}
