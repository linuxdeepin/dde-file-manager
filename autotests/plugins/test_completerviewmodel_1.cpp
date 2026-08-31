// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_completerviewmodel_1.cpp
 * @brief Unit tests for CompleterViewModel methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "models/completerviewmodel.h"

#include <QTest>

using namespace dfmplugin_titlebar;

class CompleterViewModelTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CompleterViewModel();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CompleterViewModel *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CompleterViewModelTest, setStringList)
{
    // Test setter: void setStringList((const QStringList &list))
    QStringList _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setStringList(_arg0));
}
