// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_extensionemblemmanagerprivate.cpp
 * @brief Unit tests for ExtensionEmblemManagerPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "extensionimpl/emblemimpl/extensionemblemmanager.h"

#include <QTest>

using namespace dfmplugin_utils;

class ExtensionEmblemManagerPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ExtensionEmblemManagerPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ExtensionEmblemManagerPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ExtensionEmblemManagerPrivateTest, clearReadyLocalPath)
{
    // Test method: void clearReadyLocalPath(())
    EXPECT_NO_FATAL_FAILURE(obj->clearReadyLocalPath());
}

TEST_F(ExtensionEmblemManagerPrivateTest, makeIcon)
{
    // Test method: QIcon makeIcon((const QString &path))
    QString _arg0{};
    auto result = obj->makeIcon(_arg0);
    EXPECT_TRUE(result.isNull());

}
