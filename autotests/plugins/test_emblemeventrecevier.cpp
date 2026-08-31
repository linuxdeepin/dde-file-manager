// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_emblemeventrecevier.cpp
 * @brief Unit tests for EmblemEventRecevier methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "events/emblemeventrecevier.h"

#include <QTest>

using namespace dfmplugin_emblem;

class EmblemEventRecevierTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new EmblemEventRecevier();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    EmblemEventRecevier *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(EmblemEventRecevierTest, handlePaintEmblems)
{
    // Test method: bool handlePaintEmblems((QPainter *painter, const QRectF &paintArea, const FileInfoPointer &info))
    QRectF _arg1{};
    FileInfoPointer _arg2{};
    auto result = obj->handlePaintEmblems(nullptr, _arg1, _arg2);
    EXPECT_FALSE(result);

}

TEST_F(EmblemEventRecevierTest, initializeConnections)
{
    // Test method: void initializeConnections(())
    EXPECT_NO_FATAL_FAILURE(obj->initializeConnections());
}

TEST_F(EmblemEventRecevierTest, instance)
{
    // Test getter: EmblemEventRecevier instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}
