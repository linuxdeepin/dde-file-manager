// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_filedialoghandledbus.cpp
 * @brief Unit tests for FileDialogHandleDBus methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dbus/filedialoghandledbus.h"

#include <QTest>

using namespace core;

class FileDialogHandleDBusTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileDialogHandleDBus();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileDialogHandleDBus *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileDialogHandleDBusTest, activateWindow)
{
    // Test method: void activateWindow(())
    EXPECT_NO_FATAL_FAILURE(obj->activateWindow());
}

TEST_F(FileDialogHandleDBusTest, directory)
{
    // Test getter: QString directory()
    auto result = obj->directory();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(FileDialogHandleDBusTest, directoryUrl)
{
    // Test getter: QString directoryUrl()
    auto result = obj->directoryUrl();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(FileDialogHandleDBusTest, filter)
{
    // Test getter: int filter()
    auto result = obj->filter();
    EXPECT_EQ(result, 0);

}

TEST_F(FileDialogHandleDBusTest, labelText)
{
    // Test method: QString labelText((int label))
    auto result = obj->labelText(0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(FileDialogHandleDBusTest, options)
{
    // Test getter: int options()
    auto result = obj->options();
    EXPECT_EQ(result, 0);

}

TEST_F(FileDialogHandleDBusTest, setFilter)
{
    // Test setter: void setFilter((int filters))
    EXPECT_NO_FATAL_FAILURE(obj->setFilter(0));
}

TEST_F(FileDialogHandleDBusTest, setWindowFlags)
{
    // Test setter: void setWindowFlags((quint32 windowFlags))
    EXPECT_NO_FATAL_FAILURE(obj->setWindowFlags(0));
}

TEST_F(FileDialogHandleDBusTest, setWindowTitle)
{
    // Test setter: void setWindowTitle((const QString &title))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setWindowTitle(_arg0));
}

TEST_F(FileDialogHandleDBusTest, windowFlags)
{
    // Test getter: quint32 windowFlags()
    auto result = obj->windowFlags();
    EXPECT_EQ(result, 0);

}

TEST_F(FileDialogHandleDBusTest, FileDialogHandleDBus)
{
    // Test constructor: FileDialogHandleDBus((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}
