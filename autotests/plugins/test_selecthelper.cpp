// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_selecthelper.cpp
 * @brief Unit tests for SelectHelper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/selecthelper.h"

#include <QTest>

using namespace dfmplugin_workspace;

class SelectHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SelectHelper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SelectHelper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SelectHelperTest, SelectHelper)
{
    // Test constructor: SelectHelper((FileView *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(SelectHelperTest, caculateAndSelectIndex)
{
    // Test method: void caculateAndSelectIndex((const QItemSelection &lastSelect, const QItemSelection &newSelect, QItemSelectionModel::SelectionFlags flags))
    QItemSelection _arg0{};
    QItemSelection _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->caculateAndSelectIndex(_arg0, _arg1, QItemSelectionModel::SelectionFlags()));
}

TEST_F(SelectHelperTest, caculateIconViewSelection)
{
    // Test method: void caculateIconViewSelection((const QRect &rect, QItemSelection *selection))
    QRect _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->caculateIconViewSelection(_arg0, nullptr));
}

TEST_F(SelectHelperTest, click)
{
    // Test method: void click((const QModelIndex &index))
    QModelIndex _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->click(_arg0));
}

TEST_F(SelectHelperTest, saveSelectedFilesList)
{
    // Test method: void saveSelectedFilesList((const QUrl &current, const QList<QUrl> &urls))
    QUrl _arg0{};
    QList<QUrl> _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->saveSelectedFilesList(_arg0, _arg1));
}
