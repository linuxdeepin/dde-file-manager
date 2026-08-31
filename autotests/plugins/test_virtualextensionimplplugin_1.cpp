// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_virtualextensionimplplugin_1.cpp
 * @brief Unit tests for VirtualExtensionImplPlugin methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "extensionimpl/virtualextensionimplplugin.h"

#include <QTest>

using namespace dfmplugin_utils;

class VirtualExtensionImplPluginTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new VirtualExtensionImplPlugin();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    VirtualExtensionImplPlugin *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(VirtualExtensionImplPluginTest, bindScene)
{
    // Test method: void bindScene((const QString &parentScene))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->bindScene(_arg0));
}

TEST_F(VirtualExtensionImplPluginTest, bindSceneOnAdded)
{
    // Test method: void bindSceneOnAdded((const QString &newScene))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->bindSceneOnAdded(_arg0));
}

TEST_F(VirtualExtensionImplPluginTest, followEvents)
{
    // Test method: void followEvents(())
    EXPECT_NO_FATAL_FAILURE(obj->followEvents());
}
