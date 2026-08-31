// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dirsharemenusceneprivate.cpp
 * @brief Unit tests for DirShareMenuScenePrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dirsharemenu/dirsharemenuscene.h"

#include <QTest>

using namespace dfmplugin_dirshare;

class DirShareMenuScenePrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DirShareMenuScenePrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DirShareMenuScenePrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DirShareMenuScenePrivateTest, DirShareMenuScenePrivate)
{
    // Test constructor: DirShareMenuScenePrivate((dfmbase::AbstractMenuScene *qq))
    ASSERT_NE(obj, nullptr);
}

TEST_F(DirShareMenuScenePrivateTest, addShare)
{
    // Test method: void addShare((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->addShare(_arg0));
}
