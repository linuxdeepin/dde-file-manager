// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "markdownbrowser.h"

#include <gtest/gtest.h>

#include <QDir>
#include <QFrame>
#include <QPointer>
#include <QTemporaryDir>
#include <QTextCursor>
#include <QTextDocument>
#include <QWidget>

using namespace plugin_filepreview;

class UT_MarkdownBrowser : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        browser = new MarkdownBrowser();
    }

    virtual void TearDown() override
    {
        delete browser;
        browser = nullptr;
        stub.clear();
    }

protected:
    MarkdownBrowser *browser { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(UT_MarkdownBrowser, Constructor_DefaultArguments_ReadOnlyWithFixedGeometry)
{
    EXPECT_TRUE(browser->isReadOnly());
    EXPECT_TRUE(browser->textInteractionFlags() & Qt::TextSelectableByMouse);
    EXPECT_TRUE(browser->textInteractionFlags() & Qt::TextSelectableByKeyboard);
    EXPECT_TRUE(browser->textInteractionFlags() & Qt::LinksAccessibleByMouse);
    EXPECT_TRUE(browser->openExternalLinks());
    EXPECT_EQ(browser->lineWrapMode(), QTextEdit::WidgetWidth);
    EXPECT_EQ(browser->size(), QSize(800, 500));
    EXPECT_EQ(browser->frameShape(), QFrame::NoFrame);
    EXPECT_EQ(browser->parentWidget(), nullptr);
}

TEST_F(UT_MarkdownBrowser, Constructor_WithParentWidget_ParentAssigned)
{
    QWidget parent;
    auto child = new MarkdownBrowser(&parent);
    EXPECT_EQ(child->parentWidget(), &parent);
    EXPECT_TRUE(child->isReadOnly());
    delete child;
}

TEST_F(UT_MarkdownBrowser, SetMarkdownContent_ExistingBasePath_SetsSearchPathsAndDocumentUrl)
{
    QTemporaryDir baseDir;
    ASSERT_TRUE(baseDir.isValid());

    browser->setMarkdownContent("# Unit Heading\n\nBrowser body text", baseDir.path());

    EXPECT_EQ(browser->searchPaths(), QStringList { baseDir.path() });
    EXPECT_EQ(browser->document()->metaInformation(QTextDocument::DocumentUrl),
              QUrl::fromLocalFile(baseDir.path()).toString());
    EXPECT_TRUE(browser->document()->toPlainText().contains("Unit Heading"));
    EXPECT_TRUE(browser->document()->toPlainText().contains("Browser body text"));
}

TEST_F(UT_MarkdownBrowser, SetMarkdownContent_EmptyBasePath_KeepsSearchPathsEmpty)
{
    browser->setMarkdownContent("# NoBase", "");

    EXPECT_TRUE(browser->searchPaths().isEmpty());
    EXPECT_TRUE(browser->document()->toPlainText().contains("NoBase"));
    EXPECT_FALSE(browser->document()->isEmpty());
}

TEST_F(UT_MarkdownBrowser, SetMarkdownContent_NonExistingBasePath_KeepsSearchPathsEmpty)
{
    QString ghostPath;
    {
        QTemporaryDir removedDir;
        ghostPath = removedDir.path();
    }
    ASSERT_FALSE(QDir(ghostPath).exists());

    browser->setMarkdownContent("# GhostBase", ghostPath);

    EXPECT_TRUE(browser->searchPaths().isEmpty());
    EXPECT_TRUE(browser->document()->toPlainText().contains("GhostBase"));
}

TEST_F(UT_MarkdownBrowser, SetMarkdownContent_EmptyMarkdown_ClearsPreviousDocument)
{
    QTemporaryDir baseDir;
    ASSERT_TRUE(baseDir.isValid());

    browser->setMarkdownContent("# First content", baseDir.path());
    ASSERT_TRUE(browser->document()->toPlainText().contains("First content"));

    browser->setMarkdownContent("", "");

    EXPECT_TRUE(browser->document()->isEmpty());
    EXPECT_TRUE(browser->document()->toPlainText().isEmpty());
    EXPECT_EQ(browser->searchPaths(), QStringList { baseDir.path() });
}

TEST_F(UT_MarkdownBrowser, SetMarkdownContent_CursorAtEnd_MovesCursorToDocumentStart)
{
    browser->setMarkdownContent("# First\n\npara one", "");
    browser->moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);
    ASSERT_GT(browser->textCursor().position(), 0);

    browser->setMarkdownContent("# Second\n\npara two", "");

    EXPECT_EQ(browser->textCursor().position(), 0);
    EXPECT_TRUE(browser->document()->toPlainText().contains("Second"));
}

TEST_F(UT_MarkdownBrowser, Destructor_DeleteBrowser_InvalidatesQPointer)
{
    auto disposable = new MarkdownBrowser();
    QPointer<MarkdownBrowser> guard(disposable);
    ASSERT_FALSE(guard.isNull());

    delete disposable;

    EXPECT_TRUE(guard.isNull());
}
