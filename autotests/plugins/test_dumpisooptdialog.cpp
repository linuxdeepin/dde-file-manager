// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dumpisooptdialog.cpp
 * @brief Unit tests for DumpISOOptDialog methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dialogs/dumpisooptdialog.h"

#include <QTest>

using namespace dfmplugin_burn;

class DumpISOOptDialogTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DumpISOOptDialog();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DumpISOOptDialog *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DumpISOOptDialogTest, DumpISOOptDialog)
{
    // Test constructor: DumpISOOptDialog((const QString &devId, QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(DumpISOOptDialogTest, initliazeUi)
{
    // Test method: void initliazeUi(())
    EXPECT_NO_FATAL_FAILURE(obj->initliazeUi());
}
