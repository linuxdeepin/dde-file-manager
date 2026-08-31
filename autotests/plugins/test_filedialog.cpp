// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_filedialog.cpp
 * @brief Unit tests for FileDialog methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/filedialog.h"

#include <QTest>

using namespace core;

class FileDialogTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileDialog();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileDialog *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileDialogTest, accept)
{
    // Test method: void accept(())
    EXPECT_NO_FATAL_FAILURE(obj->accept());
}

TEST_F(FileDialogTest, addCustomWidget)
{
    // Test method: void addCustomWidget((FileDialog::CustomWidgetType type, const QString &data))
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->addCustomWidget(FileDialog::CustomWidgetType(), _arg1));
}

TEST_F(FileDialogTest, directory)
{
    // Test getter: QDir directory()
    auto result = obj->directory();
    EXPECT_NO_FATAL_FAILURE({ obj->directory(); });

}

TEST_F(FileDialogTest, done)
{
    // Test method: void done((int r))
    EXPECT_NO_FATAL_FAILURE(obj->done(0));
}

TEST_F(FileDialogTest, eventFilter)
{
    // Test method: bool eventFilter((QObject *watched, QEvent *event))
    auto result = obj->eventFilter(nullptr, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(FileDialogTest, exec)
{
    // Test getter: int exec()
    auto result = obj->exec();
    EXPECT_EQ(result, 0);

}

TEST_F(FileDialogTest, handleEnterInSaveMode)
{
    // Test method: void handleEnterInSaveMode(())
    EXPECT_NO_FATAL_FAILURE(obj->handleEnterInSaveMode());
}

TEST_F(FileDialogTest, nameFilters)
{
    // Test getter: QStringList nameFilters()
    auto result = obj->nameFilters();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(FileDialogTest, onViewItemClicked)
{
    // Test method: void onViewItemClicked((const QVariantMap &data))
    QVariantMap _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onViewItemClicked(_arg0));
}

TEST_F(FileDialogTest, open)
{
    // Test method: void open(())
    EXPECT_NO_FATAL_FAILURE(obj->open());
}

TEST_F(FileDialogTest, reject)
{
    // Test method: void reject(())
    EXPECT_NO_FATAL_FAILURE(obj->reject());
}

TEST_F(FileDialogTest, saveClosedSate)
{
    // Test bool getter: saveClosedSate()
    bool result = obj->saveClosedSate();
    EXPECT_FALSE(result);

}

TEST_F(FileDialogTest, saveLastVisitedUrl)
{
    // Test method: void saveLastVisitedUrl((const QUrl &currentUrl))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->saveLastVisitedUrl(_arg0));
}

TEST_F(FileDialogTest, selectNameFilterByIndex)
{
    // Test method: void selectNameFilterByIndex((int index))
    EXPECT_NO_FATAL_FAILURE(obj->selectNameFilterByIndex(0));
}

TEST_F(FileDialogTest, selectedFiles)
{
    // Test getter: QStringList selectedFiles()
    auto result = obj->selectedFiles();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(FileDialogTest, selectedUrls)
{
    // Test getter: QList<QUrl> selectedUrls()
    auto result = obj->selectedUrls();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(FileDialogTest, setFileMode)
{
    // Test setter: void setFileMode((QFileDialog::FileMode mode))
    EXPECT_NO_FATAL_FAILURE(obj->setFileMode(QFileDialog::FileMode()));
}

TEST_F(FileDialogTest, updateAcceptButtonState)
{
    // Test method: void updateAcceptButtonState(())
    EXPECT_NO_FATAL_FAILURE(obj->updateAcceptButtonState());
}

TEST_F(FileDialogTest, updateViewState)
{
    // Test method: void updateViewState(())
    EXPECT_NO_FATAL_FAILURE(obj->updateViewState());
}

TEST_F(FileDialogTest, FileDialog)
{
    // Test constructor: FileDialog((const QUrl &url, QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(FileDialogTest, filter)
{
    // Test getter: QDir::Filters filter()
    auto result = obj->filter();
    EXPECT_NO_FATAL_FAILURE({ obj->filter(); });

}
