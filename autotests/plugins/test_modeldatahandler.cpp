// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_modeldatahandler.cpp
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

TEST_F(ModelDataHandlerTest, ModelDataHandler)
{
    // Test constructor: ModelDataHandler(())
    ASSERT_NE(obj, nullptr);
}

TEST_F(ModelDataHandlerTest, acceptReset)
{
    // Test method: QList<QUrl> acceptReset((const QList<QUrl> &urls))
    QList<QUrl> _arg0{};
    auto result = obj->acceptReset(_arg0);
    EXPECT_TRUE(result.isEmpty());

}
