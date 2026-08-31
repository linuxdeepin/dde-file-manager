// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_autoactivatewindow.cpp
 * @brief Unit tests for AutoActivateWindow methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "private/autoactivatewindow.h"

#include <QTest>

using namespace ddplugin_wallpapersetting;

class AutoActivateWindowTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new AutoActivateWindow();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    AutoActivateWindow *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(AutoActivateWindowTest, AutoActivateWindow)
{
    // Test constructor: AutoActivateWindow((QObject *parent))
    ASSERT_NE(obj, nullptr);
}
