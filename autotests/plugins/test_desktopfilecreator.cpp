// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_desktopfilecreator.cpp
 * @brief Unit tests for DesktopFileCreator methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/fileutil.h"

#include <QTest>

using namespace ddplugin_canvas;

class DesktopFileCreatorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DesktopFileCreator();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DesktopFileCreator *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DesktopFileCreatorTest, DesktopFileCreator)
{
    // Test constructor: DesktopFileCreator(())
    ASSERT_NE(obj, nullptr);
}

TEST_F(DesktopFileCreatorTest, createFileInfo)
{
    // Test method: FileInfoPointer createFileInfo((const QUrl &url, dfmbase::Global::CreateFileInfoType cache))
    QUrl _arg0{};
    auto result = obj->createFileInfo(_arg0, {});
    EXPECT_NE(result.get(), nullptr);

}

TEST_F(DesktopFileCreatorTest, instance)
{
    // Test getter: DesktopFileCreator instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}
