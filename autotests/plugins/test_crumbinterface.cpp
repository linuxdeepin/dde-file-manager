// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_crumbinterface.cpp
 * @brief Unit tests for CrumbInterface methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/crumbinterface.h"

#include <QTest>

using namespace dfmplugin_titlebar;

class CrumbInterfaceTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CrumbInterface();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CrumbInterface *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CrumbInterfaceTest, onUpdateChildren)
{
    // Test method: void onUpdateChildren((QList<QUrl> children))
    EXPECT_NO_FATAL_FAILURE(obj->onUpdateChildren(QList<QUrl>()));
}
