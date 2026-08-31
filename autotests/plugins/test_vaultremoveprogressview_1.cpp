// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_vaultremoveprogressview_1.cpp
 * @brief Unit tests for VaultRemoveProgressView methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/removevaultview/vaultremoveprogressview.h"

#include <QTest>

using namespace dfmplugin_vault;

class VaultRemoveProgressViewTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new VaultRemoveProgressView();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    VaultRemoveProgressView *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(VaultRemoveProgressViewTest, btnText)
{
    // Test getter: QStringList btnText()
    auto result = obj->btnText();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(VaultRemoveProgressViewTest, buttonClicked)
{
    // Test method: void buttonClicked((int index, const QString &text))
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->buttonClicked(0, _arg1));
}

TEST_F(VaultRemoveProgressViewTest, titleText)
{
    // Test getter: QString titleText()
    auto result = obj->titleText();
    EXPECT_TRUE(result.isEmpty());

}
