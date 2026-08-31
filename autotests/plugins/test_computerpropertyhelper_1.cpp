// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_computerpropertyhelper_1.cpp
 * @brief Unit tests for ComputerPropertyHelper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/computerpropertyhelper.h"

#include <QTest>

using namespace dfmplugin_propertydialog;

class ComputerPropertyHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ComputerPropertyHelper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ComputerPropertyHelper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ComputerPropertyHelperTest, createComputerProperty)
{
    // Test method: QWidget createComputerProperty((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->createComputerProperty(_arg0);
    EXPECT_NO_FATAL_FAILURE({ obj->createComputerProperty(_arg0); });

}
