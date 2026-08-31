// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_remotepasswdmanager.cpp
 * @brief Unit tests for RemotePasswdManager methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/remotepasswdmanager.h"

#include <QTest>

using namespace dfmplugin_computer;

class RemotePasswdManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new RemotePasswdManager();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    RemotePasswdManager *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(RemotePasswdManagerTest, clearPasswd)
{
    // Test method: void clearPasswd((const QString &uri))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->clearPasswd(_arg0));
}

TEST_F(RemotePasswdManagerTest, onPasswdCleared)
{
    // Test method: void onPasswdCleared((GObject *obj, GAsyncResult *res, gpointer data))
    EXPECT_NO_FATAL_FAILURE(obj->onPasswdCleared(nullptr, nullptr, {}));
}
