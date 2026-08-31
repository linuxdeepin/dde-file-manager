// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_modeldatahandler_1.cpp
 * @brief Unit tests for ModelDataHandler methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "models/modeldatahandler.h"

#include <QTest>

using namespace ddplugin_organizer;

class ModelDataHandlerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ModelDataHandler();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ModelDataHandler *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ModelDataHandlerTest, acceptInsert)
{
    // Test method: bool acceptInsert((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->acceptInsert(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(ModelDataHandlerTest, acceptRename)
{
    // Test method: bool acceptRename((const QUrl &oldUrl, const QUrl &newUrl))
    QUrl _arg0{};
    QUrl _arg1{};
    auto result = obj->acceptRename(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(ModelDataHandlerTest, acceptUpdate)
{
    // Test method: bool acceptUpdate((const QUrl &url, const QVector<int> &roles))
    QUrl _arg0{};
    QVector<int> _arg1{};
    auto result = obj->acceptUpdate(_arg0, _arg1);
    EXPECT_FALSE(result);

}
