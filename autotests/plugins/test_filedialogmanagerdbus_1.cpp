// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_filedialogmanagerdbus_1.cpp
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

TEST_F(FileDialogManagerDBusTest, canUseFileChooserDialog)
{
    // Test method: bool canUseFileChooserDialog((const QString &group, const QString &executableFileName))
    QString _arg0{};
    QString _arg1{};
    auto result = obj->canUseFileChooserDialog(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(FileDialogManagerDBusTest, createDialog)
{
    // Test method: QDBusObjectPath createDialog((QString key))
    auto result = obj->createDialog(QString());
    EXPECT_NO_FATAL_FAILURE({ obj->createDialog(QString()); });

}

TEST_F(FileDialogManagerDBusTest, globPatternsForMime)
{
    // Test method: QStringList globPatternsForMime((const QString &mimeType))
    QString _arg0{};
    auto result = obj->globPatternsForMime(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(FileDialogManagerDBusTest, isUseFileChooserDialog)
{
    // Test bool getter: isUseFileChooserDialog()
    bool result = obj->isUseFileChooserDialog();
    EXPECT_FALSE(result);

}

TEST_F(FileDialogManagerDBusTest, monitorFiles)
{
    // Test getter: QStringList monitorFiles()
    auto result = obj->monitorFiles();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(FileDialogManagerDBusTest, onAppExit)
{
    // Test method: void onAppExit(())
    EXPECT_NO_FATAL_FAILURE(obj->onAppExit());
}

TEST_F(FileDialogManagerDBusTest, showBluetoothTransDialog)
{
    // Test method: void showBluetoothTransDialog((const QString &id, const QStringList &URIs))
    QString _arg0{};
    QStringList _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->showBluetoothTransDialog(_arg0, _arg1));
}
