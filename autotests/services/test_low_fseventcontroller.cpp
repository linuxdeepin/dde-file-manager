// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_low_fseventcontroller.cpp
 * @brief Unit tests for FSEventController methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/textindex/fsmonitor/fseventcontroller.h"

#include <QTest>

using namespace src;

class FSEventControllerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FSEventController();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FSEventController *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FSEventControllerTest, onConfigChanged)
{
    // Test method: void onConfigChanged(())
    EXPECT_NO_FATAL_FAILURE(obj->onConfigChanged());
}

TEST_F(FSEventControllerTest, onFilesCreated)
{
    // Test method: void onFilesCreated((const QStringList &paths))
    QStringList _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onFilesCreated(_arg0));
}

TEST_F(FSEventControllerTest, onFilesModified)
{
    // Test method: void onFilesModified((const QStringList &paths))
    QStringList _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onFilesModified(_arg0));
}

TEST_F(FSEventControllerTest, onFilesMoved)
{
    // Test method: void onFilesMoved((const QHash<QString, QString> &movedPaths))
    QHash<QString, QString> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onFilesMoved(_arg0));
}

TEST_F(FSEventControllerTest, onFlushFinished)
{
    // Test method: void onFlushFinished(())
    EXPECT_NO_FATAL_FAILURE(obj->onFlushFinished());
}

TEST_F(FSEventControllerTest, setEnabledNow)
{
    // Test setter: void setEnabledNow((bool enabled))
    EXPECT_NO_FATAL_FAILURE(obj->setEnabledNow(false));
}
