// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_filedialoghandledbus_1.cpp
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

TEST_F(FileDialogHandleDBusTest, acceptMode)
{
    // Test getter: int acceptMode()
    auto result = obj->acceptMode();
    EXPECT_EQ(result, 0);

}

TEST_F(FileDialogHandleDBusTest, heartbeatInterval)
{
    // Test getter: int heartbeatInterval()
    auto result = obj->heartbeatInterval();
    EXPECT_EQ(result, 0);

}

TEST_F(FileDialogHandleDBusTest, makeHeartbeat)
{
    // Test method: void makeHeartbeat(())
    EXPECT_NO_FATAL_FAILURE(obj->makeHeartbeat());
}

TEST_F(FileDialogHandleDBusTest, selectUrl)
{
    // Test method: void selectUrl((const QString &url))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->selectUrl(_arg0));
}

TEST_F(FileDialogHandleDBusTest, selectedUrls)
{
    // Test getter: QStringList selectedUrls()
    auto result = obj->selectedUrls();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(FileDialogHandleDBusTest, setAcceptMode)
{
    // Test setter: void setAcceptMode((int mode))
    EXPECT_NO_FATAL_FAILURE(obj->setAcceptMode(0));
}

TEST_F(FileDialogHandleDBusTest, setDirectoryUrl)
{
    // Test setter: void setDirectoryUrl((const QString &directory))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setDirectoryUrl(_arg0));
}

TEST_F(FileDialogHandleDBusTest, setFileMode)
{
    // Test setter: void setFileMode((int mode))
    EXPECT_NO_FATAL_FAILURE(obj->setFileMode(0));
}

TEST_F(FileDialogHandleDBusTest, setHeartbeatInterval)
{
    // Test setter: void setHeartbeatInterval((int interval))
    EXPECT_NO_FATAL_FAILURE(obj->setHeartbeatInterval(0));
}

TEST_F(FileDialogHandleDBusTest, setLabelText)
{
    // Test setter: void setLabelText((int label, const QString &text))
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->setLabelText(0, _arg1));
}

TEST_F(FileDialogHandleDBusTest, setOption)
{
    // Test setter: void setOption((int option, bool on))
    EXPECT_NO_FATAL_FAILURE(obj->setOption(0, false));
}

TEST_F(FileDialogHandleDBusTest, setOptions)
{
    // Test setter: void setOptions((int options))
    EXPECT_NO_FATAL_FAILURE(obj->setOptions(0));
}

TEST_F(FileDialogHandleDBusTest, setViewMode)
{
    // Test setter: void setViewMode((int mode))
    EXPECT_NO_FATAL_FAILURE(obj->setViewMode(0));
}

TEST_F(FileDialogHandleDBusTest, testOption)
{
    // Test method: bool testOption((int option))
    auto result = obj->testOption(0);
    EXPECT_FALSE(result);

}

TEST_F(FileDialogHandleDBusTest, viewMode)
{
    // Test getter: int viewMode()
    auto result = obj->viewMode();
    EXPECT_EQ(result, 0);

}

TEST_F(FileDialogHandleDBusTest, winId)
{
    // Test getter: qulonglong winId()
    auto result = obj->winId();
    EXPECT_EQ(result, 0);

}

TEST_F(FileDialogHandleDBusTest, windowActive)
{
    // Test bool getter: windowActive()
    bool result = obj->windowActive();
    EXPECT_FALSE(result);

}
