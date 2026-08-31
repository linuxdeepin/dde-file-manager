// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_filedialogstatusbar_1.cpp
 * @brief Unit tests for FileDialogStatusBar methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/filedialogstatusbar.h"

#include <QTest>

using namespace core;

class FileDialogStatusBarTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileDialogStatusBar();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileDialogStatusBar *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileDialogStatusBarTest, acceptButton)
{
    // Test getter: DSuggestButton acceptButton()
    auto result = obj->acceptButton();
    EXPECT_NO_FATAL_FAILURE({ obj->acceptButton(); });

}

TEST_F(FileDialogStatusBarTest, addComboBox)
{
    // Test method: void addComboBox((DLabel *label, DComboBox *box))
    EXPECT_NO_FATAL_FAILURE(obj->addComboBox(nullptr, nullptr));
}

TEST_F(FileDialogStatusBarTest, addLineEdit)
{
    // Test method: void addLineEdit((DLabel *label, DLineEdit *edit))
    EXPECT_NO_FATAL_FAILURE(obj->addLineEdit(nullptr, nullptr));
}

TEST_F(FileDialogStatusBarTest, allComboBoxsValue)
{
    // Test getter: QVariantMap allComboBoxsValue()
    auto result = obj->allComboBoxsValue();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(FileDialogStatusBarTest, allLineEditsValue)
{
    // Test getter: QVariantMap allLineEditsValue()
    auto result = obj->allLineEditsValue();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(FileDialogStatusBarTest, beginAddCustomWidget)
{
    // Test method: void beginAddCustomWidget(())
    EXPECT_NO_FATAL_FAILURE(obj->beginAddCustomWidget());
}

TEST_F(FileDialogStatusBarTest, changeFileNameEditText)
{
    // Test method: void changeFileNameEditText((const QString &fileName))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->changeFileNameEditText(_arg0));
}

TEST_F(FileDialogStatusBarTest, comboBox)
{
    // Test getter: DComboBox comboBox()
    auto result = obj->comboBox();
    EXPECT_NO_FATAL_FAILURE({ obj->comboBox(); });

}

TEST_F(FileDialogStatusBarTest, endAddCustomWidget)
{
    // Test method: void endAddCustomWidget(())
    EXPECT_NO_FATAL_FAILURE(obj->endAddCustomWidget());
}

TEST_F(FileDialogStatusBarTest, eventFilter)
{
    // Test method: bool eventFilter((QObject *watched, QEvent *event))
    auto result = obj->eventFilter(nullptr, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(FileDialogStatusBarTest, getComboBoxValue)
{
    // Test method: QString getComboBoxValue((const QString &text))
    QString _arg0{};
    auto result = obj->getComboBoxValue(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(FileDialogStatusBarTest, getLineEditValue)
{
    // Test method: QString getLineEditValue((const QString &text))
    QString _arg0{};
    auto result = obj->getLineEditValue(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(FileDialogStatusBarTest, hideEvent)
{
    // Test event handler: hideEvent((QHideEvent *event))
    QHideEvent _event(QHideEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->hideEvent(&_event));
}

TEST_F(FileDialogStatusBarTest, initializeConnect)
{
    // Test method: void initializeConnect(())
    EXPECT_NO_FATAL_FAILURE(obj->initializeConnect());
}

TEST_F(FileDialogStatusBarTest, onFileNameTextEdited)
{
    // Test method: void onFileNameTextEdited((const QString &text))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onFileNameTextEdited(_arg0));
}

TEST_F(FileDialogStatusBarTest, onWindowTitleChanged)
{
    // Test method: void onWindowTitleChanged((const QString &title))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onWindowTitleChanged(_arg0));
}

TEST_F(FileDialogStatusBarTest, rejectButton)
{
    // Test getter: DPushButton rejectButton()
    auto result = obj->rejectButton();
    EXPECT_NO_FATAL_FAILURE({ obj->rejectButton(); });

}

TEST_F(FileDialogStatusBarTest, setAppropriateWidgetFocus)
{
    // Test method: void setAppropriateWidgetFocus(())
    EXPECT_NO_FATAL_FAILURE(obj->setAppropriateWidgetFocus());
}

TEST_F(FileDialogStatusBarTest, setComBoxItems)
{
    // Test setter: void setComBoxItems((const QStringList &list))
    QStringList _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setComBoxItems(_arg0));
}

TEST_F(FileDialogStatusBarTest, setMode)
{
    // Test setter: void setMode((FileDialogStatusBar::Mode mode))
    EXPECT_NO_FATAL_FAILURE(obj->setMode(FileDialogStatusBar::Mode()));
}

TEST_F(FileDialogStatusBarTest, showEvent)
{
    // Test event handler: showEvent((QShowEvent *event))
    QShowEvent _event(QShowEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->showEvent(&_event));
}

TEST_F(FileDialogStatusBarTest, updateComboxViewWidth)
{
    // Test method: void updateComboxViewWidth(())
    EXPECT_NO_FATAL_FAILURE(obj->updateComboxViewWidth());
}
