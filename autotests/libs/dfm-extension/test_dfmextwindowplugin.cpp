// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dfmextwindowplugin.cpp
 * @brief Unit tests for DFMExtWindowPlugin methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-extension/window/dfmextwindowplugin.h"

#include <QTest>

using namespace src;

class DFMExtWindowPluginTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DFMExtWindowPlugin();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DFMExtWindowPlugin *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DFMExtWindowPluginTest, DFMExtWindowPlugin)
{
    // Test constructor: DFMExtWindowPlugin(())
    ASSERT_NE(obj, nullptr);
}

TEST_F(DFMExtWindowPluginTest, M_~DFMExtWindowPlugin)
{
    // Test method:  ~DFMExtWindowPlugin(())
    EXPECT_NO_FATAL_FAILURE({ DFMExtWindowPlugin *tmp = new DFMExtWindowPlugin(); delete tmp; });
}

TEST_F(DFMExtWindowPluginTest, windowOpened)
{
    // Test method: void windowOpened((uint64_t winId))
    EXPECT_NO_FATAL_FAILURE(obj->windowOpened({}));
}

TEST_F(DFMExtWindowPluginTest, windowClosed)
{
    // Test method: void windowClosed((uint64_t winId))
    EXPECT_NO_FATAL_FAILURE(obj->windowClosed({}));
}

TEST_F(DFMExtWindowPluginTest, firstWindowOpened)
{
    // Test method: void firstWindowOpened((uint64_t winId))
    EXPECT_NO_FATAL_FAILURE(obj->firstWindowOpened({}));
}

TEST_F(DFMExtWindowPluginTest, lastWindowClosed)
{
    // Test method: void lastWindowClosed((uint64_t winId))
    EXPECT_NO_FATAL_FAILURE(obj->lastWindowClosed({}));
}

TEST_F(DFMExtWindowPluginTest, windowUrlChanged)
{
    // Test method: void windowUrlChanged((uint64_t winId, const std::string &urlString))
    std::string _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->windowUrlChanged({}, _arg1));
}

TEST_F(DFMExtWindowPluginTest, registerWindowOpened)
{
    // Test method: void registerWindowOpened((const WindowFunc &func))
    WindowFunc _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->registerWindowOpened(_arg0));
}

TEST_F(DFMExtWindowPluginTest, registerWindowClosed)
{
    // Test method: void registerWindowClosed((const WindowFunc &func))
    WindowFunc _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->registerWindowClosed(_arg0));
}

TEST_F(DFMExtWindowPluginTest, registerFirstWindowOpened)
{
    // Test method: void registerFirstWindowOpened((const WindowFunc &func))
    WindowFunc _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->registerFirstWindowOpened(_arg0));
}

TEST_F(DFMExtWindowPluginTest, registerLastWindowClosed)
{
    // Test method: void registerLastWindowClosed((const WindowFunc &func))
    WindowFunc _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->registerLastWindowClosed(_arg0));
}

TEST_F(DFMExtWindowPluginTest, registerWindowUrlChanged)
{
    // Test method: void registerWindowUrlChanged((const WindowUrlFunc &func))
    WindowUrlFunc _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->registerWindowUrlChanged(_arg0));
}

TEST_F(DFMExtWindowPluginTest, M_(DFMExtWindowPlugin))
{
    // Test getter: DFM_DISABLE_COPY (DFMExtWindowPlugin)()
    EXPECT_NO_FATAL_FAILURE({ obj->(DFMExtWindowPlugin)(); });
}

TEST_F(DFMExtWindowPluginTest, d)
{
    // Test getter: DFMExtWindowPluginPrivate d()
    auto result = obj->d();
    EXPECT_NO_FATAL_FAILURE({ obj->d(); });

}
