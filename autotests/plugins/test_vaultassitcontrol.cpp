// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_vaultassitcontrol.cpp
 * @brief Unit tests for VaultAssitControl methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "vaultassist/vaultassitcontrol.h"

#include <QTest>

using namespace dfmplugin_utils;

class VaultAssitControlTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new VaultAssitControl();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    VaultAssitControl *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(VaultAssitControlTest, scheme)
{
    // Test getter: QString scheme()
    auto result = obj->scheme();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(VaultAssitControlTest, transUrlsToLocal)
{
    // Test method: QList<QUrl> transUrlsToLocal((const QList<QUrl> &urls))
    QList<QUrl> _arg0{};
    auto result = obj->transUrlsToLocal(_arg0);
    EXPECT_TRUE(result.isEmpty());

}
