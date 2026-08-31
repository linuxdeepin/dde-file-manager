// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_prehandler_utils.cpp
 * @brief Unit tests for prehandler_utils methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "events/traversprehandler.h"

#include <QTest>

using namespace dfmplugin_smbbrowser;

class prehandler_utilsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new prehandler_utils();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    prehandler_utils *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(prehandler_utilsTest, splitMountSource)
{
    // Test method: QString splitMountSource((const QString &source, QString *subPath))
    QString _arg0{};
    auto result = obj->splitMountSource(_arg0, nullptr);
    EXPECT_TRUE(result.isEmpty());

}
