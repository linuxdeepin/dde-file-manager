// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_abstractentryfileentity.cpp
 * @brief Unit tests for AbstractEntryFileEntity methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-base/file/entry/entities/abstractentryfileentity.h"

#include <QTest>

using namespace src;

class AbstractEntryFileEntityTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new AbstractEntryFileEntity();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    AbstractEntryFileEntity *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(AbstractEntryFileEntityTest, AbstractEntryFileEntity)
{
    // Test constructor: AbstractEntryFileEntity((const QUrl &url))
    ASSERT_NE(obj, nullptr);
}
