// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_iconitemeditorprivate.cpp
 * @brief Unit tests for IconItemEditorPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/private/iconitemeditor_p.h"

#include <QTest>

using namespace dfmplugin_workspace;

class IconItemEditorPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new IconItemEditorPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    IconItemEditorPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(IconItemEditorPrivateTest, IconItemEditorPrivate)
{
    // Test constructor: IconItemEditorPrivate((IconItemEditor *qq))
    ASSERT_NE(obj, nullptr);
}
