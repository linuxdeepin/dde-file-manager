// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_virtualextensionimplplugin.cpp
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

TEST_F(VirtualExtensionImplPluginTest, initialize)
{
    // Test getter: DFMBASE_USE_NAMESPACE initialize()
    EXPECT_NO_FATAL_FAILURE({ obj->initialize(); });
}

TEST_F(VirtualExtensionImplPluginTest, start)
{
    // Test bool getter: start()
    bool result = obj->start();
    EXPECT_FALSE(result);

}
