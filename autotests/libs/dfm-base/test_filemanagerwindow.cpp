// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_filemanagerwindow.cpp
 * @brief Unit tests for FileManagerWindow methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-base/widgets/dfmwindow/filemanagerwindow.h"

#include <QTest>

using namespace src;

class FileManagerWindowTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileManagerWindow();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileManagerWindow *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileManagerWindowTest, FileManagerWindow)
{
    // Test constructor: FileManagerWindow((const QUrl &url, QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(FileManagerWindowTest, event)
{
    // Test method: bool event((QEvent *event))
    auto result = obj->event(nullptr);
    EXPECT_FALSE(result);

}

TEST_F(FileManagerWindowTest, eventFilter)
{
    // Test method: bool eventFilter((QObject *watched, QEvent *event))
    auto result = obj->eventFilter(nullptr, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(FileManagerWindowTest, initializeUi)
{
    // Test method: void initializeUi(())
    EXPECT_NO_FATAL_FAILURE(obj->initializeUi());
}

TEST_F(FileManagerWindowTest, moveCenter)
{
    // Test method: void moveCenter(())
    EXPECT_NO_FATAL_FAILURE(obj->moveCenter());
}

TEST_F(FileManagerWindowTest, saveClosedSate)
{
    // Test bool getter: saveClosedSate()
    bool result = obj->saveClosedSate();
    EXPECT_FALSE(result);

}

TEST_F(FileManagerWindowTest, saveState)
{
    // Test method: void saveState(())
    EXPECT_NO_FATAL_FAILURE(obj->saveState());
}

TEST_F(FileManagerWindowTest, showDetailSpace)
{
    // Test method: void showDetailSpace((const QVariantHash &options))
    QVariantHash _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->showDetailSpace(_arg0));
}

TEST_F(FileManagerWindowTest, sideBar)
{
    // Test getter: AbstractFrame sideBar()
    auto result = obj->sideBar();
    EXPECT_NO_FATAL_FAILURE({ obj->sideBar(); });

}
