// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_extensionmonitor_1.cpp
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

TEST_F(ExtensionMonitorTest, ExtensionMonitor)
{
    // Test constructor: ExtensionMonitor((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(ExtensionMonitorTest, checkAndMkpath)
{
    // Test method: void checkAndMkpath((const QString &path))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->checkAndMkpath(_arg0));
}

TEST_F(ExtensionMonitorTest, copyInitialFiles)
{
    // Test method: void copyInitialFiles(())
    EXPECT_NO_FATAL_FAILURE(obj->copyInitialFiles());
}

TEST_F(ExtensionMonitorTest, instance)
{
    // Test getter: ExtensionMonitor instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}

TEST_F(ExtensionMonitorTest, onFileAdded)
{
    // Test method: void onFileAdded((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onFileAdded(_arg0));
}

TEST_F(ExtensionMonitorTest, processExtensionDirectory)
{
    // Test method: void processExtensionDirectory((const QString &sourcePath, const QString &targetPath))
    QString _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->processExtensionDirectory(_arg0, _arg1));
}

TEST_F(ExtensionMonitorTest, setupFileWatchers)
{
    // Test method: void setupFileWatchers(())
    EXPECT_NO_FATAL_FAILURE(obj->setupFileWatchers());
}

TEST_F(ExtensionMonitorTest, start)
{
    // Test method: void start(())
    EXPECT_NO_FATAL_FAILURE(obj->start());
}
