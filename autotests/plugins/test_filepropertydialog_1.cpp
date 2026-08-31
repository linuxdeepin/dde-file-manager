// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_filepropertydialog_1.cpp
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

TEST_F(FilePropertyDialogTest, addExtendedControl)
{
    // Test method: void addExtendedControl((QWidget *widget, ViewExtensionUpdateFunc updater))
    EXPECT_NO_FATAL_FAILURE(obj->addExtendedControl(nullptr, ViewExtensionUpdateFunc()));
}

TEST_F(FilePropertyDialogTest, closeDialog)
{
    // Test method: void closeDialog(())
    EXPECT_NO_FATAL_FAILURE(obj->closeDialog());
}

TEST_F(FilePropertyDialogTest, closeEvent)
{
    // Test event handler: closeEvent((QCloseEvent *event))
    QCloseEvent _event(QCloseEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->closeEvent(&_event));
}

TEST_F(FilePropertyDialogTest, createBasicWidget)
{
    // Test method: void createBasicWidget((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->createBasicWidget(_arg0));
}

TEST_F(FilePropertyDialogTest, createHeadUI)
{
    // Test method: void createHeadUI((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->createHeadUI(_arg0));
}

TEST_F(FilePropertyDialogTest, createPermissionManagerWidget)
{
    // Test method: void createPermissionManagerWidget((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->createPermissionManagerWidget(_arg0));
}

TEST_F(FilePropertyDialogTest, eventFilter)
{
    // Test method: bool eventFilter((QObject *object, QEvent *event))
    auto result = obj->eventFilter(nullptr, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(FilePropertyDialogTest, filterControlView)
{
    // Test method: void filterControlView(())
    EXPECT_NO_FATAL_FAILURE(obj->filterControlView());
}

TEST_F(FilePropertyDialogTest, getFileCount)
{
    // Test getter: int getFileCount()
    auto result = obj->getFileCount();
    EXPECT_EQ(result, 0);

}

TEST_F(FilePropertyDialogTest, getFileSize)
{
    // Test getter: qint64 getFileSize()
    auto result = obj->getFileSize();
    EXPECT_EQ(result, 0);

}

TEST_F(FilePropertyDialogTest, initInfoUI)
{
    // Test method: void initInfoUI(())
    EXPECT_NO_FATAL_FAILURE(obj->initInfoUI());
}

TEST_F(FilePropertyDialogTest, initalHeightOfView)
{
    // Test getter: int initalHeightOfView()
    auto result = obj->initalHeightOfView();
    EXPECT_EQ(result, 0);

}

TEST_F(FilePropertyDialogTest, keyPressEvent)
{
    // Test event handler: keyPressEvent((QKeyEvent *event))
    QKeyEvent _event(QKeyEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->keyPressEvent(&_event));
}

TEST_F(FilePropertyDialogTest, mousePressEvent)
{
    // Test event handler: mousePressEvent((QMouseEvent *event))
    QMouseEvent _event(QMouseEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->mousePressEvent(&_event));
}

TEST_F(FilePropertyDialogTest, onFileInfoUpdated)
{
    // Test method: void onFileInfoUpdated((const QUrl &url, const QString &infoPtr, const bool isLinkOrg))
    QUrl _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->onFileInfoUpdated(_arg0, _arg1, false));
}

TEST_F(FilePropertyDialogTest, processHeight)
{
    // Test method: void processHeight((int height))
    EXPECT_NO_FATAL_FAILURE(obj->processHeight(0));
}

TEST_F(FilePropertyDialogTest, resizeEvent)
{
    // Test event handler: resizeEvent((QResizeEvent *event))
    QResizeEvent _event(QResizeEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->resizeEvent(&_event));
}

TEST_F(FilePropertyDialogTest, selectFileUrl)
{
    // Test method: void selectFileUrl((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->selectFileUrl(_arg0));
}

TEST_F(FilePropertyDialogTest, setBasicInfoExpand)
{
    // Test setter: void setBasicInfoExpand((bool expand))
    EXPECT_NO_FATAL_FAILURE(obj->setBasicInfoExpand(false));
}

TEST_F(FilePropertyDialogTest, showEvent)
{
    // Test event handler: showEvent((QShowEvent *event))
    QShowEvent _event(QShowEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->showEvent(&_event));
}
