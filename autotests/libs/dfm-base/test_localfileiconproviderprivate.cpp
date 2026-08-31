// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_localfileiconproviderprivate.cpp
 * @brief Unit tests for LocalFileIconProviderPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-base/file/local/localfileiconprovider.h"

#include <QTest>

using namespace src;

class LocalFileIconProviderPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new LocalFileIconProviderPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    LocalFileIconProviderPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(LocalFileIconProviderPrivateTest, fromTheme)
{
    // Test method: QIcon fromTheme((QString iconName))
    auto result = obj->fromTheme(QString());
    EXPECT_TRUE(result.isNull());

}
