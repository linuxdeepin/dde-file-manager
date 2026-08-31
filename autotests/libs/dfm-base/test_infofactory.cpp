// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_infofactory.cpp
 * @brief Unit tests for InfoFactory methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-base/base/schemefactory.h"

#include <QTest>

using namespace include;

class InfoFactoryTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new InfoFactory();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    InfoFactory *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(InfoFactoryTest, getFileInfoFromCache)
{
    // Test method: QSharedPointer<FileInfo> getFileInfoFromCache((const QUrl &url, Global::CreateFileInfoType type, QString *errorString))
    QUrl _arg0{};
    auto result = obj->getFileInfoFromCache(_arg0, Global::CreateFileInfoType(), nullptr);
    EXPECT_NE(result.get(), nullptr);

}

TEST_F(InfoFactoryTest, scheme)
{
    // Test method: QString scheme((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->scheme(_arg0);
    EXPECT_TRUE(result.isEmpty());

}
