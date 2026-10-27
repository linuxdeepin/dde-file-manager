// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "markdownbrowser.h"
#include "markdownrenderer.h"
#include "markdowntheme.h"

#include <DGuiApplicationHelper>

#include <gtest/gtest.h>

#include <QCoreApplication>
#include <QDir>
#include <QFont>
#include <QFrame>
#include <QPointer>
#include <QScrollBar>
#include <QTemporaryDir>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextOption>
#include <QWidget>

DGUI_USE_NAMESPACE

using namespace plugin_filepreview;

namespace {

QTextBlock blockContaining(const QTextDocument &document, const QString &needle)
{
    for (QTextBlock block = document.begin(); block.isValid(); block = block.next()) {
        if (block.text().contains(needle))
            return block;
    }
    return QTextBlock();
}

QString longMarkdown()
{
    QString markdown;
    for (int i = 0; i < 200; ++i) {
        markdown += QStringLiteral("Paragraph %1 with enough text to wrap over several lines.\n\n")
                            .arg(i);
    }
    return markdown;
}

}   // namespace

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

TEST_F(UT_MarkdownBrowser, SetMarkdownContent_AppliesThemeDocumentMetrics)
{
    const MarkdownTheme theme = MarkdownTheme::fromApplication();

    browser->setMarkdownContent(QStringLiteral("plain text"), QString());

    EXPECT_EQ(browser->document()->documentMargin(), theme.documentMargin);
    EXPECT_DOUBLE_EQ(browser->document()->defaultFont().pointSizeF(),
                     theme.baseFontPointSize);
    EXPECT_EQ(browser->document()->defaultTextOption().wrapMode(),
              QTextOption::WrapAtWordBoundaryOrAnywhere);
}

TEST_F(UT_MarkdownBrowser, SetMarkdownContent_ExistingBasePath_SetsDocumentBaseUrl)
{
    QTemporaryDir baseDir;
    ASSERT_TRUE(baseDir.isValid());

    browser->setMarkdownContent("# heading", baseDir.path());

    // The trailing slash keeps relative resources inside the folder.
    EXPECT_EQ(browser->document()->baseUrl(),
              QUrl::fromLocalFile(baseDir.path() + QLatin1Char('/')));
}

TEST_F(UT_MarkdownBrowser, SetMarkdownContent_MarkdownBlocks_CarryThemeFormatting)
{
    browser->setMarkdownContent(QStringLiteral("# Unit Heading\n\n```cpp\nint value = 0;\n```"),
                                QString());

    const QTextBlock heading = blockContaining(*browser->document(), QStringLiteral("Unit Heading"));
    ASSERT_TRUE(heading.isValid());
    EXPECT_GE(heading.charFormat().fontWeight(), int(QFont::Bold));

    const QTextBlock code = blockContaining(*browser->document(), QStringLiteral("int value"));
    ASSERT_TRUE(code.isValid());
    EXPECT_TRUE(MarkdownRenderer::isCodeBlock(code, MarkdownTheme::fromApplication()));
}

TEST_F(UT_MarkdownBrowser, OnThemeTypeChanged_WithoutContent_KeepsDocumentEmpty)
{
    browser->onThemeTypeChanged();

    EXPECT_TRUE(browser->document()->isEmpty());
    EXPECT_TRUE(browser->document()->toPlainText().isEmpty());
}

TEST_F(UT_MarkdownBrowser, OnThemeTypeChanged_WithContent_RebuildsStoredMarkdown)
{
    browser->setMarkdownContent(QStringLiteral("# Original"), QString());
    ASSERT_TRUE(browser->document()->toPlainText().contains(QStringLiteral("Original")));

    browser->m_markdown = QStringLiteral("# Rebuilt for the new theme");
    browser->onThemeTypeChanged();

    const QString text = browser->document()->toPlainText();
    EXPECT_TRUE(text.contains(QStringLiteral("Rebuilt for the new theme")));
    EXPECT_FALSE(text.contains(QStringLiteral("Original")));
}

TEST_F(UT_MarkdownBrowser, ThemeTypeChangedSignal_TriggersRebuildThroughConstructorConnection)
{
    browser->setMarkdownContent(QStringLiteral("# Original"), QString());
    ASSERT_TRUE(browser->document()->toPlainText().contains(QStringLiteral("Original")));

    browser->m_markdown = QStringLiteral("# Rebuilt after signal");
    Q_EMIT DGuiApplicationHelper::instance()->themeTypeChanged(DGuiApplicationHelper::LightType);

    EXPECT_TRUE(browser->document()->toPlainText().contains(QStringLiteral("Rebuilt after signal")));
    EXPECT_FALSE(browser->document()->toPlainText().contains(QStringLiteral("Original")));
}

TEST_F(UT_MarkdownBrowser, RenderContent_KeepingScrollPosition_DoesNotJumpToTop)
{
    browser->setMarkdownContent(longMarkdown(), QString());
    browser->show();
    QCoreApplication::processEvents();

    QScrollBar *bar = browser->verticalScrollBar();
    if (bar->maximum() <= 0)
        GTEST_SKIP() << "vertical scroll bar has no range in this environment";

    bar->setValue(bar->maximum() / 2);
    const int expected = bar->value();
    ASSERT_GT(expected, 0);

    browser->renderContent(false);

    EXPECT_EQ(bar->value(), expected);
}

TEST_F(UT_MarkdownBrowser, RenderContent_ResettingScroll_ReturnsToDocumentStart)
{
    browser->setMarkdownContent(longMarkdown(), QString());
    browser->show();
    QCoreApplication::processEvents();

    browser->verticalScrollBar()->setValue(browser->verticalScrollBar()->maximum());
    browser->moveCursor(QTextCursor::End);
    ASSERT_GT(browser->textCursor().position(), 0);

    browser->renderContent(true);

    EXPECT_EQ(browser->verticalScrollBar()->value(), 0);
    EXPECT_EQ(browser->textCursor().position(), 0);
}
