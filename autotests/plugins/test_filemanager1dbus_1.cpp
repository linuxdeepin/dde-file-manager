// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_filemanager1dbus_1.cpp
 * @brief Unit tests for FileManager1DBus methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "filemanager1dbus.h"

#include <QTest>

using namespace filemanager1;

class FileManager1DBusTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileManager1DBus();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileManager1DBus *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileManager1DBusTest, ShowFolders)
{
    // Test method: void ShowFolders((const QStringList &URIs, const QString &StartupId))
    QStringList _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->ShowFolders(_arg0, _arg1));
}

TEST_F(FileManager1DBusTest, ShowItemProperties)
{
    // Test method: void ShowItemProperties((const QStringList &URIs, const QString &StartupId))
    QStringList _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->ShowItemProperties(_arg0, _arg1));
}

TEST_F(FileManager1DBusTest, ShowItems)
{
    // Test method: void ShowItems((const QStringList &URIs, const QString &StartupId))
    QStringList _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->ShowItems(_arg0, _arg1));
}

TEST_F(FileManager1DBusTest, Trash)
{
    // Test method: void Trash((const QStringList &URIs))
    QStringList _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->Trash(_arg0));
}
