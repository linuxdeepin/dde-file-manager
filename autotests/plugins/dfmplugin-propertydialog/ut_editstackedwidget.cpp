// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>
#include <QTemporaryFile>
#include <QTemporaryDir>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QFocusEvent>

#include "stubext.h"
#include "dfmplugin_propertydialog_global.h"
#include "views/editstackedwidget.h"

#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/file/local/syncfileinfo.h>

DPPROPERTYDIALOG_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

namespace {


}   // namespace

class EditStackedWidgetImpl : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
        UrlRoute::regScheme(Global::Scheme::kFile, "/");
        InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
        tempDir.reset(new QTemporaryDir);
        tempFile.reset(new QTemporaryFile(tempDir->path() + "/editXXXXXX.txt"));
        tempFile->open();
    }

    void TearDown() override
    {
        stub.clear();
        tempFile.reset();
        tempDir.reset();
    }

    stub_ext::StubExt stub;
    std::unique_ptr<QTemporaryDir> tempDir;
    std::unique_ptr<QTemporaryFile> tempFile;
};

TEST_F(EditStackedWidgetImpl, NameTextEditConstruct)
{
    NameTextEdit *edit = new NameTextEdit("hello");
    EXPECT_NE(edit, nullptr);
    EXPECT_EQ(edit->toPlainText(), "hello");
    delete edit;
}

TEST_F(EditStackedWidgetImpl, NameTextEditIsCanceled)
{
    NameTextEdit edit("test");
    EXPECT_FALSE(edit.isCanceled());
    edit.setIsCanceled(true);
    EXPECT_TRUE(edit.isCanceled());
    edit.setIsCanceled(false);
    EXPECT_FALSE(edit.isCanceled());
}

TEST_F(EditStackedWidgetImpl, NameTextEditSetPlainTextAndSlotChanged)
{
    NameTextEdit edit("old");
    edit.setPlainText("new name");
    EXPECT_EQ(edit.toPlainText(), "new name");
    EXPECT_NO_THROW(edit.slotTextChanged());
}

TEST_F(EditStackedWidgetImpl, NameTextEditFocusOutEvent)
{
    NameTextEdit edit("test");
    QFocusEvent event(QEvent::FocusOut);
    EXPECT_NO_THROW(edit.focusOutEvent(&event));
}

TEST_F(EditStackedWidgetImpl, NameTextEditKeyPressEventEscape)
{
    NameTextEdit edit("test");
    QKeyEvent event(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
    EXPECT_NO_THROW(edit.keyPressEvent(&event));
    EXPECT_TRUE(edit.isCanceled());
}

TEST_F(EditStackedWidgetImpl, NameTextEditKeyPressEventReturn)
{
    NameTextEdit edit("test");
    QKeyEvent event(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
    EXPECT_NO_THROW(edit.keyPressEvent(&event));
    EXPECT_FALSE(edit.isCanceled());
}

TEST_F(EditStackedWidgetImpl, ConstructDestruct)
{
    EditStackedWidget *widget = new EditStackedWidget();
    EXPECT_NE(widget, nullptr);
    delete widget;
}

TEST_F(EditStackedWidgetImpl, InitTextShowFrame)
{
    EditStackedWidget widget;
    EXPECT_NO_THROW(widget.initTextShowFrame("some file name.txt"));
}

TEST_F(EditStackedWidgetImpl, SelectFile)
{
    QUrl url = QUrl::fromLocalFile(tempFile->fileName());

    EditStackedWidget widget;
    EXPECT_NO_THROW(widget.selectFile(url));
}

TEST_F(EditStackedWidgetImpl, RenameFile)
{
    QUrl url = QUrl::fromLocalFile(tempFile->fileName());

    EditStackedWidget widget;
    widget.selectFile(url);
    EXPECT_NO_THROW(widget.renameFile());
}

TEST_F(EditStackedWidgetImpl, ShowTextShowFrameSameName)
{
    QUrl url = QUrl::fromLocalFile(tempFile->fileName());

    EditStackedWidget widget;
    widget.selectFile(url);
    widget.renameFile();

    QString name = QFileInfo(tempFile->fileName()).fileName();
    widget.findChild<NameTextEdit *>()->setPlainText(name);
    EXPECT_NO_THROW(widget.showTextShowFrame());
}

TEST_F(EditStackedWidgetImpl, MouseProcess)
{
    QUrl url = QUrl::fromLocalFile(tempFile->fileName());

    EditStackedWidget widget;
    widget.selectFile(url);
    widget.renameFile();

    QMouseEvent event(QEvent::MouseButtonPress, QPointF(5, 5), Qt::LeftButton, Qt::LeftButton,
                      Qt::NoModifier);
    EXPECT_NO_THROW(widget.mouseProcess(&event));
}
