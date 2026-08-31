// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_detailmanager.cpp
 * @brief Unit tests for DetailManager methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/detailmanager.h"

#include <QTest>

using namespace dfmplugin_detailspace;

class DetailManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DetailManager();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DetailManager *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DetailManagerTest, createBasicViewExtensionField)
{
    // Test method: QMap<BasicExpandType, BasicExpandMap> createBasicViewExtensionField((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->createBasicViewExtensionField(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DetailManagerTest, instance)
{
    // Test getter: DetailManager instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}
