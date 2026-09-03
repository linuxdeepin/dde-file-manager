// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <stub-ext/stubext.h>

#include "../../../src/plugins/filedialog/core/views/filedialogstatusbar.h"
#include "../../../src/plugins/filedialog/core/views/filedialog.h"
#include "../../../src/plugins/filedialog/core/views/filedialog_p.h"

#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-base/base/schemefactory.h>

#include <QApplication>
#include <QShowEvent>
#include <QHideEvent>
#include <QFocusEvent>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>

#include <DLineEdit>
#include <DLabel>
#include <DComboBox>
#include <DSuggestButton>
#include <DPushButton>

DWIDGET_USE_NAMESPACE
using namespace filedialog_core;

class FileDialogStatusBarImpl : public testing::Test
{
protected:
    void SetUp() override
    {
        if (!qApp) {
            int argc = 0;
            char **argv = nullptr;
            new QApplication(argc, argv);
        }

        using FindWinFunc = dfmbase::FileManagerWindow *(FileManagerWindowsManager::*)(quint64);
        stub.set_lamda(static_cast<FindWinFunc>(&FileManagerWindowsManager::findWindowById),
                       [](FileManagerWindowsManager *, quint64) -> dfmbase::FileManagerWindow * {
                           __DBG_STUB_INVOKE__
                           return nullptr;
                       });

        dialog = new FileDialog(QUrl::fromLocalFile("/tmp"));
        statusBar = new FileDialogStatusBar(dialog);
    }

    void TearDown() override
    {
        delete dialog;   // deletes child statusBar
        dialog = nullptr;
        statusBar = nullptr;
        stub.clear();
    }

    FileDialog *dialog { nullptr };
    FileDialogStatusBar *statusBar { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(FileDialogStatusBarImpl, ConstructorCreatesWidgets)
{
    EXPECT_NE(statusBar, nullptr);
    EXPECT_NE(statusBar->comboBox(), nullptr);
    EXPECT_NE(statusBar->lineEdit(), nullptr);
    EXPECT_NE(statusBar->acceptButton(), nullptr);
    EXPECT_NE(statusBar->rejectButton(), nullptr);
}
TEST_F(FileDialogStatusBarImpl, SetComBoxItems)
{
    QStringList items { "Images (*.png)", "Text Files (*.txt)" };
    statusBar->setComBoxItems(items);
    EXPECT_EQ(statusBar->comboBox()->count(), 2);
}

TEST_F(FileDialogStatusBarImpl, LineEditRoundTrip)
{
    DLabel *label = new DLabel("Label");
    DLineEdit *edit = new DLineEdit();
    edit->setText("value");
    statusBar->addLineEdit(label, edit);

    EXPECT_EQ(statusBar->getLineEditValue("Label"), QString("value"));
    QVariantMap all = statusBar->allLineEditsValue();
    EXPECT_TRUE(all.contains("Label"));
    EXPECT_EQ(all.value("Label").toString(), QString("value"));
    EXPECT_TRUE(statusBar->getLineEditValue("Missing").isEmpty());

    delete label;
    delete edit;
}

TEST_F(FileDialogStatusBarImpl, ComboBoxRoundTrip)
{
    DLabel *label = new DLabel("Format");
    DComboBox *box = new DComboBox();
    box->addItems({ "pdf", "txt" });
    box->setCurrentText("txt");
    statusBar->addComboBox(label, box);

    EXPECT_EQ(statusBar->getComboBoxValue("Format"), QString("txt"));
    QVariantMap all = statusBar->allComboBoxsValue();
    EXPECT_TRUE(all.contains("Format"));
    EXPECT_EQ(all.value("Format").toString(), QString("txt"));
    EXPECT_TRUE(statusBar->getComboBoxValue("Missing").isEmpty());

    delete label;
    delete box;
}

TEST_F(FileDialogStatusBarImpl, BeginEndAddCustomWidgetClearsLists)
{
    DLabel *l = new DLabel("L");
    DLineEdit *e = new DLineEdit();
    statusBar->addLineEdit(l, e);

    statusBar->beginAddCustomWidget();
    statusBar->endAddCustomWidget();

    EXPECT_TRUE(statusBar->allLineEditsValue().isEmpty());

    delete l;
    delete e;
}

TEST_F(FileDialogStatusBarImpl, ChangeFileNameEditTextKeepsSuffix)
{
    statusBar->lineEdit()->setText("old.txt");
    statusBar->changeFileNameEditText("new");
    EXPECT_EQ(statusBar->lineEdit()->text(), QString("new.txt"));
}
TEST_F(FileDialogStatusBarImpl, EventFilterHandlesFocusAndShow)
{
    QFocusEvent focusIn(QEvent::FocusIn);
    QApplication::sendEvent(statusBar->lineEdit(), &focusIn);
    EXPECT_TRUE(true);

    QShowEvent showEvent;
    QApplication::sendEvent(statusBar->lineEdit(), &showEvent);
    EXPECT_TRUE(true);
}

TEST_F(FileDialogStatusBarImpl, OnFileNameTextEditedDoesNotCrash)
{
    dialog->d.data()->currentUrl = QUrl::fromLocalFile("/tmp");
    statusBar->lineEdit()->setText("name");
    statusBar->onFileNameTextEdited("hello/world");
    EXPECT_TRUE(true);
}

TEST_F(FileDialogStatusBarImpl, UpdateComboxViewWidthDoesNotCrash)
{
    statusBar->updateComboxViewWidth();
    EXPECT_TRUE(true);
}

TEST_F(FileDialogStatusBarImpl, SetAppropriateWidgetFocusDoesNotCrash)
{
    statusBar->setMode(FileDialogStatusBar::kSave);
    statusBar->setComBoxItems({});
    statusBar->setAppropriateWidgetFocus();
    EXPECT_TRUE(true);
}
