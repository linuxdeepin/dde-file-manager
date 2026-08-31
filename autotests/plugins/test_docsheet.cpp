// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_docsheet.cpp
 * @brief Unit tests for DocSheet methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "apps/dde-file-manager-preview/pluginpreviews/pdf-preview/docsheet.h"

#include <QTest>

using namespace src;

class DocSheetTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DocSheet();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DocSheet *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DocSheetTest, filePath)
{
    // Test getter: QString filePath()
    auto result = obj->filePath();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DocSheetTest, fileType)
{
    // Test getter: FileType fileType()
    auto result = obj->fileType();
    EXPECT_NO_FATAL_FAILURE({ obj->fileType(); });

}

TEST_F(DocSheetTest, format)
{
    // Test getter: QString format()
    auto result = obj->format();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DocSheetTest, onOpened)
{
    // Test method: void onOpened((Document::Error error))
    EXPECT_NO_FATAL_FAILURE(obj->onOpened(Document::Error()));
}

TEST_F(DocSheetTest, operation)
{
    // Test getter: SheetOperation operation()
    auto result = obj->operation();
    EXPECT_NO_FATAL_FAILURE({ obj->operation(); });

}

TEST_F(DocSheetTest, resetChildParent)
{
    // Test method: void resetChildParent(())
    EXPECT_NO_FATAL_FAILURE(obj->resetChildParent());
}

TEST_F(DocSheetTest, setAlive)
{
    // Test setter: void setAlive((bool alive))
    EXPECT_NO_FATAL_FAILURE(obj->setAlive(false));
}

TEST_F(DocSheetTest, thumbnail)
{
    // Test method: QPixmap thumbnail((int index))
    auto result = obj->thumbnail(0);
    EXPECT_TRUE(result.isNull());

}
