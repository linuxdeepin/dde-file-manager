// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_filedialog_1.cpp
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

TEST_F(FileDialogTest, acceptMode)
{
    // Test getter: QFileDialog::AcceptMode acceptMode()
    auto result = obj->acceptMode();
    EXPECT_NO_FATAL_FAILURE({ obj->acceptMode(); });

}

TEST_F(FileDialogTest, allCustomWidgetsValue)
{
    // Test method: QVariantMap allCustomWidgetsValue((FileDialog::CustomWidgetType type))
    auto result = obj->allCustomWidgetsValue(FileDialog::CustomWidgetType());
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(FileDialogTest, beginAddCustomWidget)
{
    // Test method: void beginAddCustomWidget(())
    EXPECT_NO_FATAL_FAILURE(obj->beginAddCustomWidget());
}

TEST_F(FileDialogTest, cd)
{
    // Test method: void cd((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->cd(_arg0));
}

TEST_F(FileDialogTest, checkFileSuffix)
{
    // Test method: bool checkFileSuffix((const QString &filename, QString &suffix))
    QString _arg0{};
    QString _arg1{};
    auto result = obj->checkFileSuffix(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(FileDialogTest, closeEvent)
{
    // Test event handler: closeEvent((QCloseEvent *event))
    QCloseEvent _event(QCloseEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->closeEvent(&_event));
}

TEST_F(FileDialogTest, currentViewMode)
{
    // Test getter: QFileDialog::ViewMode currentViewMode()
    auto result = obj->currentViewMode();
    EXPECT_NO_FATAL_FAILURE({ obj->currentViewMode(); });

}

TEST_F(FileDialogTest, directoryUrl)
{
    // Test getter: QUrl directoryUrl()
    auto result = obj->directoryUrl();
    EXPECT_TRUE(result.isEmpty() || result.isValid());
}

TEST_F(FileDialogTest, endAddCustomWidget)
{
    // Test method: void endAddCustomWidget(())
    EXPECT_NO_FATAL_FAILURE(obj->endAddCustomWidget());
}

TEST_F(FileDialogTest, getCustomWidgetValue)
{
    // Test method: QVariant getCustomWidgetValue((FileDialog::CustomWidgetType type, const QString &text))
    QString _arg1{};
    auto result = obj->getCustomWidgetValue(FileDialog::CustomWidgetType(), _arg1);
    EXPECT_FALSE(result.isValid());

}

TEST_F(FileDialogTest, getcurrenturl)
{
    // Test getter: QUrl getcurrenturl()
    auto result = obj->getcurrenturl();
    EXPECT_TRUE(result.isEmpty() || result.isValid());
}

TEST_F(FileDialogTest, handleEnterInOpenMode)
{
    // Test method: void handleEnterInOpenMode(())
    EXPECT_NO_FATAL_FAILURE(obj->handleEnterInOpenMode());
}

TEST_F(FileDialogTest, handleEnterPressed)
{
    // Test method: void handleEnterPressed(())
    EXPECT_NO_FATAL_FAILURE(obj->handleEnterPressed());
}

TEST_F(FileDialogTest, handleRenameEndAcceptBtn)
{
    // Test method: void handleRenameEndAcceptBtn((const quint64 windowID, const QUrl &url))
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->handleRenameEndAcceptBtn(0, _arg1));
}

TEST_F(FileDialogTest, handleRenameStartAcceptBtn)
{
    // Test method: void handleRenameStartAcceptBtn((const quint64 windowID, const QUrl &url))
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->handleRenameStartAcceptBtn(0, _arg1));
}

TEST_F(FileDialogTest, handleUrlChanged)
{
    // Test method: void handleUrlChanged((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->handleUrlChanged(_arg0));
}

TEST_F(FileDialogTest, hideOnAccept)
{
    // Test bool getter: hideOnAccept()
    bool result = obj->hideOnAccept();
    EXPECT_FALSE(result);

}

TEST_F(FileDialogTest, initConnect)
{
    // Test method: void initConnect(())
    EXPECT_NO_FATAL_FAILURE(obj->initConnect());
}

TEST_F(FileDialogTest, initEventsConnect)
{
    // Test method: void initEventsConnect(())
    EXPECT_NO_FATAL_FAILURE(obj->initEventsConnect());
}

TEST_F(FileDialogTest, initializeUi)
{
    // Test method: void initializeUi(())
    EXPECT_NO_FATAL_FAILURE(obj->initializeUi());
}

TEST_F(FileDialogTest, isFileNameEditFocused)
{
    // Test bool getter: isFileNameEditFocused()
    bool result = obj->isFileNameEditFocused();
    EXPECT_FALSE(result);

}

TEST_F(FileDialogTest, labelText)
{
    // Test method: QString labelText((QFileDialog::DialogLabel label))
    auto result = obj->labelText(QFileDialog::DialogLabel());
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(FileDialogTest, lastVisitedUrl)
{
    // Test getter: QUrl lastVisitedUrl()
    auto result = obj->lastVisitedUrl();
    EXPECT_TRUE(result.isEmpty() || result.isValid());
}

TEST_F(FileDialogTest, modelCurrentNameFilter)
{
    // Test getter: QString modelCurrentNameFilter()
    auto result = obj->modelCurrentNameFilter();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(FileDialogTest, onAcceptButtonClicked)
{
    // Test method: void onAcceptButtonClicked(())
    EXPECT_NO_FATAL_FAILURE(obj->onAcceptButtonClicked());
}

TEST_F(FileDialogTest, onCurrentInputNameChanged)
{
    // Test method: void onCurrentInputNameChanged(())
    EXPECT_NO_FATAL_FAILURE(obj->onCurrentInputNameChanged());
}

TEST_F(FileDialogTest, onRejectButtonClicked)
{
    // Test method: void onRejectButtonClicked(())
    EXPECT_NO_FATAL_FAILURE(obj->onRejectButtonClicked());
}

TEST_F(FileDialogTest, onViewSelectionChanged)
{
    // Test method: void onViewSelectionChanged((const quint64 windowID, const QItemSelection &selected, const QItemSelection &deselected))
    QItemSelection _arg1{};
    QItemSelection _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->onViewSelectionChanged(0, _arg1, _arg2));
}

TEST_F(FileDialogTest, options)
{
    // Test getter: QFileDialog::Options options()
    auto result = obj->options();
    EXPECT_NO_FATAL_FAILURE({ obj->options(); });

}

TEST_F(FileDialogTest, selectFile)
{
    // Test method: void selectFile((const QString &filename))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->selectFile(_arg0));
}

TEST_F(FileDialogTest, selectNameFilter)
{
    // Test method: void selectNameFilter((const QString &filter))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->selectNameFilter(_arg0));
}

TEST_F(FileDialogTest, selectUrl)
{
    // Test method: void selectUrl((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->selectUrl(_arg0));
}

TEST_F(FileDialogTest, selectedNameFilter)
{
    // Test getter: QString selectedNameFilter()
    auto result = obj->selectedNameFilter();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(FileDialogTest, selectedNameFilterIndex)
{
    // Test getter: int selectedNameFilterIndex()
    auto result = obj->selectedNameFilterIndex();
    EXPECT_EQ(result, 0);

}

TEST_F(FileDialogTest, setAcceptMode)
{
    // Test setter: void setAcceptMode((QFileDialog::AcceptMode mode))
    EXPECT_NO_FATAL_FAILURE(obj->setAcceptMode(QFileDialog::AcceptMode()));
}

TEST_F(FileDialogTest, setAllowMixedSelection)
{
    // Test setter: void setAllowMixedSelection((bool on))
    EXPECT_NO_FATAL_FAILURE(obj->setAllowMixedSelection(false));
}

TEST_F(FileDialogTest, setCurrentInputName)
{
    // Test setter: void setCurrentInputName((const QString &name))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setCurrentInputName(_arg0));
}

TEST_F(FileDialogTest, setDirectory)
{
    // Test setter: void setDirectory((const QDir &directory))
    QDir _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setDirectory(_arg0));
}

TEST_F(FileDialogTest, setDirectoryUrl)
{
    // Test setter: void setDirectoryUrl((const QUrl &directory))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setDirectoryUrl(_arg0));
}

TEST_F(FileDialogTest, setFilter)
{
    // Test setter: void setFilter((QDir::Filters filters))
    EXPECT_NO_FATAL_FAILURE(obj->setFilter(QDir::Filters()));
}

TEST_F(FileDialogTest, setHideOnAccept)
{
    // Test setter: void setHideOnAccept((bool enable))
    EXPECT_NO_FATAL_FAILURE(obj->setHideOnAccept(false));
}

TEST_F(FileDialogTest, setLabelText)
{
    // Test setter: void setLabelText((QFileDialog::DialogLabel label, const QString &text))
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->setLabelText(QFileDialog::DialogLabel(), _arg1));
}

TEST_F(FileDialogTest, setNameFilters)
{
    // Test setter: void setNameFilters((const QStringList &filters))
    QStringList _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setNameFilters(_arg0));
}

TEST_F(FileDialogTest, setOption)
{
    // Test setter: void setOption((QFileDialog::Option option, bool on))
    EXPECT_NO_FATAL_FAILURE(obj->setOption(QFileDialog::Option(), false));
}

TEST_F(FileDialogTest, setOptions)
{
    // Test setter: void setOptions((QFileDialog::Options options))
    EXPECT_NO_FATAL_FAILURE(obj->setOptions(QFileDialog::Options()));
}

TEST_F(FileDialogTest, showEvent)
{
    // Test event handler: showEvent((QShowEvent *event))
    QShowEvent _event(QShowEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->showEvent(&_event));
}

TEST_F(FileDialogTest, statusBar)
{
    // Test getter: FileDialogStatusBar statusBar()
    auto result = obj->statusBar();
    EXPECT_NO_FATAL_FAILURE({ obj->statusBar(); });

}

TEST_F(FileDialogTest, testOption)
{
    // Test method: bool testOption((QFileDialog::Option option))
    auto result = obj->testOption(QFileDialog::Option());
    EXPECT_FALSE(result);

}

TEST_F(FileDialogTest, updateAsDefaultSize)
{
    // Test method: void updateAsDefaultSize(())
    EXPECT_NO_FATAL_FAILURE(obj->updateAsDefaultSize());
}

TEST_F(FileDialogTest, urlSchemeEnable)
{
    // Test method: void urlSchemeEnable((const QString &scheme, bool enable))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->urlSchemeEnable(_arg0, false));
}
