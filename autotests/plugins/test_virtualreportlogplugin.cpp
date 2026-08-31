// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_virtualreportlogplugin.cpp
 * @brief Unit tests for VirtualReportLogPlugin methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "reportlog/virtualreportlogplugin.h"

#include <QTest>

using namespace dfmplugin_utils;

class VirtualReportLogPluginTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new VirtualReportLogPlugin();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    VirtualReportLogPlugin *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(VirtualReportLogPluginTest, initialize)
{
    // Test method: void initialize(())
    EXPECT_NO_FATAL_FAILURE(obj->initialize());
}

TEST_F(VirtualReportLogPluginTest, start)
{
    // Test bool getter: start()
    bool result = obj->start();
    EXPECT_FALSE(result);

}
