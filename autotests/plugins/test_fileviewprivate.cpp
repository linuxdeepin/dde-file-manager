// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fileviewprivate.cpp
 * @brief Unit tests for FileViewPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/private/fileview_p.h"

#include <QTest>

using namespace dfmplugin_workspace;

class FileViewPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileViewPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileViewPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileViewPrivateTest, initListModeView)
{
    // Test method: void initListModeView(())
    EXPECT_NO_FATAL_FAILURE(obj->initListModeView());
}

TEST_F(FileViewPrivateTest, loadViewMode)
{
    // Test method: void loadViewMode((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->loadViewMode(_arg0));
}

TEST_F(FileViewPrivateTest, shouldPersistState)
{
    // Test method: bool shouldPersistState((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->shouldPersistState(_arg0);
    EXPECT_FALSE(result);

}
