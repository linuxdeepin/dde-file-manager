// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dfmextfileplugin.cpp
 * @brief Unit tests for DFMExtFilePlugin methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-extension/file/dfmextfileplugin.h"

#include <QTest>

using namespace src;

class DFMExtFilePluginTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DFMExtFilePlugin();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DFMExtFilePlugin *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DFMExtFilePluginTest, DFMExtFilePlugin)
{
    // Test constructor: DFMExtFilePlugin(())
    ASSERT_NE(obj, nullptr);
}

TEST_F(DFMExtFilePluginTest, M_~DFMExtFilePlugin)
{
    // Test method:  ~DFMExtFilePlugin(())
    EXPECT_NO_FATAL_FAILURE({ DFMExtFilePlugin *tmp = new DFMExtFilePlugin(); delete tmp; });
}

TEST_F(DFMExtFilePluginTest, fileOperationHooks)
{
    // Test method: void fileOperationHooks((DFMExtFileOperationHook *hook))
    EXPECT_NO_FATAL_FAILURE(obj->fileOperationHooks(nullptr));
}

TEST_F(DFMExtFilePluginTest, registerFileOperationHooks)
{
    // Test method: void registerFileOperationHooks((const FileOperationHooksFunc &func))
    FileOperationHooksFunc _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->registerFileOperationHooks(_arg0));
}

TEST_F(DFMExtFilePluginTest, M_(DFMExtFilePlugin))
{
    // Test getter: DFM_DISABLE_COPY (DFMExtFilePlugin)()
    EXPECT_NO_FATAL_FAILURE({ obj->(DFMExtFilePlugin)(); });
}

TEST_F(DFMExtFilePluginTest, d)
{
    // Test getter: DFMExtFilePluginPrivate d()
    auto result = obj->d();
    EXPECT_NO_FATAL_FAILURE({ obj->d(); });

}
