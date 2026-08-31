// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_sidebarimageviewmodel.cpp
 * @brief Unit tests for SideBarImageViewModel methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "apps/dde-file-manager-preview/pluginpreviews/pdf-preview/sidebarimageviewmodel.h"

#include <QTest>

using namespace src;

class SideBarImageViewModelTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SideBarImageViewModel();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SideBarImageViewModel *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SideBarImageViewModelTest, data)
{
    // Test method: QVariant data((const QModelIndex &index, int role))
    QModelIndex _arg0{};
    auto result = obj->data(_arg0, 0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(SideBarImageViewModelTest, getModelIndexForPageIndex)
{
    // Test method: QList<QModelIndex> getModelIndexForPageIndex((int pageIndex))
    auto result = obj->getModelIndexForPageIndex(0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(SideBarImageViewModelTest, resetData)
{
    // Test method: void resetData(())
    EXPECT_NO_FATAL_FAILURE(obj->resetData());
}
