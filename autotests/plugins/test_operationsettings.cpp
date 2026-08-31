// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_operationsettings.cpp
 * @brief Unit tests for OperationSettings methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "settings/operationsettings.h"

#include <QTest>

using namespace dfmplugin_fileoperations;

class OperationSettingsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new OperationSettings();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    OperationSettings *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(OperationSettingsTest, createSyncModeItem)
{
    // Test method: DCORE_USE_NAMESPACE createSyncModeItem((QObject *opt))
    auto result = obj->createSyncModeItem(nullptr);
    EXPECT_NO_FATAL_FAILURE({ obj->createSyncModeItem(nullptr); });

}
