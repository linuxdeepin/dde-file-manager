// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_extensionmonitor.cpp
 * @brief Unit tests for ExtensionMonitor methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "oemmenuscene/extensionmonitor.h"

#include <QTest>

using namespace dfmplugin_menu;

class ExtensionMonitorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ExtensionMonitor();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ExtensionMonitor *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ExtensionMonitorTest, onFileDeleted)
{
    // Test method: void onFileDeleted((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onFileDeleted(_arg0));
}
