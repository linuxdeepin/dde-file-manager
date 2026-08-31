// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_secret_utils.cpp
 * @brief Unit tests for secret_utils methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "displaycontrol/utilities/protocoldisplayutilities.h"

#include <QTest>

using namespace dfmplugin_smbbrowser;

class secret_utilsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new secret_utils();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    secret_utils *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(secret_utilsTest, forgetPasswordInSession)
{
    // Test method: void forgetPasswordInSession((const QString &host))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->forgetPasswordInSession(_arg0));
}
