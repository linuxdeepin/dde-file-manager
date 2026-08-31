// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fileviewmenuhelper.cpp
 * @brief Unit tests for FileViewMenuHelper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/fileviewmenuhelper.h"

#include <QTest>

using namespace dfmplugin_workspace;

class FileViewMenuHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileViewMenuHelper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileViewMenuHelper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileViewMenuHelperTest, disableMenu)
{
    // Test bool getter: disableMenu()
    bool result = obj->disableMenu();
    EXPECT_FALSE(result);

}

TEST_F(FileViewMenuHelperTest, showEmptyAreaMenu)
{
    // Test method: void showEmptyAreaMenu(())
    EXPECT_NO_FATAL_FAILURE(obj->showEmptyAreaMenu());
}

TEST_F(FileViewMenuHelperTest, showNormalMenu)
{
    // Test method: void showNormalMenu((const QModelIndex &index, const Qt::ItemFlags &indexFlags))
    QModelIndex _arg0{};
    Qt::ItemFlags _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->showNormalMenu(_arg0, _arg1));
}
