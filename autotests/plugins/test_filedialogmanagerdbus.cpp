// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_filedialogmanagerdbus.cpp
 * @brief Unit tests for FileDialogManagerDBus methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dbus/filedialogmanagerdbus.h"

#include <QTest>

using namespace core;

class FileDialogManagerDBusTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileDialogManagerDBus();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileDialogManagerDBus *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileDialogManagerDBusTest, destroyDialog)
{
    // Test method: void destroyDialog((const QDBusObjectPath &path))
    QDBusObjectPath _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->destroyDialog(_arg0));
}

TEST_F(FileDialogManagerDBusTest, dialogs)
{
    // Test getter: QList<QDBusObjectPath> dialogs()
    auto result = obj->dialogs();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(FileDialogManagerDBusTest, errorString)
{
    // Test getter: QString errorString()
    auto result = obj->errorString();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(FileDialogManagerDBusTest, initEventsFilter)
{
    // Test method: void initEventsFilter(())
    EXPECT_NO_FATAL_FAILURE(obj->initEventsFilter());
}

TEST_F(FileDialogManagerDBusTest, onDialogDestroy)
{
    // Test method: void onDialogDestroy(())
    EXPECT_NO_FATAL_FAILURE(obj->onDialogDestroy());
}
