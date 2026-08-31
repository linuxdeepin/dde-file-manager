// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_previewlibrary.cpp
 * @brief Unit tests for PreviewLibrary methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "apps/dde-file-manager-preview/filepreview/previewlibrary.h"

#include <QTest>

using namespace src;

class PreviewLibraryTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new PreviewLibrary();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    PreviewLibrary *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(PreviewLibraryTest, load)
{
    // Test bool getter: load()
    bool result = obj->load();
    EXPECT_FALSE(result);

}

TEST_F(PreviewLibraryTest, unload)
{
    // Test method: void unload(())
    EXPECT_NO_FATAL_FAILURE(obj->unload());
}
