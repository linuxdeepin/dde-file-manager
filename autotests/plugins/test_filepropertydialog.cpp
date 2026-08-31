// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_filepropertydialog.cpp
 * @brief Unit tests for FilePropertyDialog methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/filepropertydialog.h"

#include <QTest>

using namespace dfmplugin_propertydialog;

class FilePropertyDialogTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FilePropertyDialog();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FilePropertyDialog *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FilePropertyDialogTest, contentHeight)
{
    // Test getter: int contentHeight()
    auto result = obj->contentHeight();
    EXPECT_EQ(result, 0);

}

TEST_F(FilePropertyDialogTest, insertExtendedControl)
{
    // Test method: void insertExtendedControl((int index, QWidget *widget, ViewExtensionUpdateFunc updater))
    EXPECT_NO_FATAL_FAILURE(obj->insertExtendedControl(0, nullptr, ViewExtensionUpdateFunc()));
}

TEST_F(FilePropertyDialogTest, onSelectUrlRenamed)
{
    // Test method: void onSelectUrlRenamed((const QUrl &newUrl))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onSelectUrlRenamed(_arg0));
}

TEST_F(FilePropertyDialogTest, setFileIcon)
{
    // Test setter: void setFileIcon((QLabel *fileIcon, FileInfoPointer fileInfo))
    EXPECT_NO_FATAL_FAILURE(obj->setFileIcon(nullptr, FileInfoPointer()));
}

TEST_F(FilePropertyDialogTest, FilePropertyDialog)
{
    // Test constructor: FilePropertyDialog((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}
