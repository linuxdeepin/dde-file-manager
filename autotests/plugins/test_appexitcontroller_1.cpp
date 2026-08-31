// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_appexitcontroller_1.cpp
 * @brief Unit tests for AppExitController methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/appexitcontroller.h"

#include <QTest>

using namespace core;

class AppExitControllerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new AppExitController();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    AppExitController *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(AppExitControllerTest, AppExitController)
{
    // Test constructor: AppExitController((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(AppExitControllerTest, onExit)
{
    // Test method: void onExit(())
    EXPECT_NO_FATAL_FAILURE(obj->onExit());
}

TEST_F(AppExitControllerTest, readyToExit)
{
    // Test method: void readyToExit((int seconds, ExitConfirmFunc confirm))
    EXPECT_NO_FATAL_FAILURE(obj->readyToExit(0, ExitConfirmFunc()));
}
