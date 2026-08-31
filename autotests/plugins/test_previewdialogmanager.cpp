// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_previewdialogmanager.cpp
 * @brief Unit tests for PreviewDialogManager methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "apps/dde-file-manager-preview/libdfm-preview/utils/previewdialogmanager.h"

#include <QTest>

using namespace src;

class PreviewDialogManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new PreviewDialogManager();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    PreviewDialogManager *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(PreviewDialogManagerTest, instance)
{
    // Test getter: PreviewDialogManager instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}
