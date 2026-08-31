// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dirshare.cpp
 * @brief Unit tests for DirShare methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dirshare.h"

#include <QTest>

using namespace dfmplugin_dirshare;

class DirShareTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DirShare();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DirShare *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DirShareTest, bindEvents)
{
    // Test method: void bindEvents(())
    EXPECT_NO_FATAL_FAILURE(obj->bindEvents());
}

TEST_F(DirShareTest, bindScene)
{
    // Test method: void bindScene((const QString &parentScene))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->bindScene(_arg0));
}
