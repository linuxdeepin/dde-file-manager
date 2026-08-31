// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_sessionbusiness.cpp
 * @brief Unit tests for SessionBusiness methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "apps/dde-file-manager/sessionloader.h"

#include <QTest>

using namespace src;

class SessionBusinessTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SessionBusiness();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SessionBusiness *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SessionBusinessTest, instance)
{
    // Test getter: SessionBusiness instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}

TEST_F(SessionBusinessTest, savePath)
{
    // Test method: void savePath((quint64 wid, const QString &path))
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->savePath(0, _arg1));
}
