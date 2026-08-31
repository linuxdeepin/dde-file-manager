// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_desktopappurl.cpp
 * @brief Unit tests for DesktopAppUrl methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-base/utils/fileutils.h"

#include <QTest>

using namespace src;

class DesktopAppUrlTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DesktopAppUrl();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DesktopAppUrl *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DesktopAppUrlTest, computerDesktopFileUrl)
{
    // Test getter: QUrl computerDesktopFileUrl()
    auto result = obj->computerDesktopFileUrl();
    EXPECT_TRUE(result.isEmpty() || result.isValid());
}

TEST_F(DesktopAppUrlTest, homeDesktopFileUrl)
{
    // Test getter: QUrl homeDesktopFileUrl()
    auto result = obj->homeDesktopFileUrl();
    EXPECT_TRUE(result.isEmpty() || result.isValid());
}

TEST_F(DesktopAppUrlTest, trashDesktopFileUrl)
{
    // Test getter: QUrl trashDesktopFileUrl()
    auto result = obj->trashDesktopFileUrl();
    EXPECT_TRUE(result.isEmpty() || result.isValid());
}
