// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "markdownrenderer.h"
#include "markdowntheme.h"

#include <gtest/gtest.h>

#include <QBrush>
#include <QColor>
#include <QFont>
#include <QTextBlock>
#include <QTextBlockFormat>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextFragment>
#include <QTextOption>
#include <QVector>

using namespace plugin_filepreview;

namespace {

MarkdownTheme renderTheme()
{
    MarkdownTheme theme;
    theme.textColor = QColor(0x11, 0x22, 0x33);
    theme.headingColor = QColor(0x22, 0x33, 0x44);
    theme.mutedColor = QColor(0x33, 0x44, 0x55);
    theme.linkColor = QColor(0x44, 0x55, 0x66);
    theme.codeTextColor = QColor(0x55, 0x66, 0x77);
    theme.codeBackground = QColor(0x66, 0x77, 0x88);
    theme.inlineCodeBackground = QColor(0x77, 0x88, 0x99);
    theme.quoteTextColor = QColor(0x88, 0x99, 0xaa);
    theme.quoteBarColor = QColor(0x99, 0xaa, 0xbb);
    theme.tableHeaderBackground = QColor(0xaa, 0xbb, 0xcc);
    theme.textFontFamily = QStringLiteral("UnitTestSans");
    theme.monoFontFamily = QStringLiteral("UnitTestMono");
    theme.baseFontPointSize = 10.0;
    theme.documentMargin = 24;
    theme.contentWidth = 700;
    return theme;
}

QTextBlock findBlock(const QTextDocument &document, const QString &needle)
{
    for (QTextBlock block = document.begin(); block.isValid(); block = block.next()) {
        if (block.text().contains(needle))
            return block;
    }
    return QTextBlock();
}

// Code blocks carry the theme's codeTextColor on every character, so a plain
// "has any foreground" check cannot tell highlighting apart from the <pre>
// surface. Look for a colour the surface did not already provide instead.
// Note that an unset QBrush still reports black, hence the style check.
bool hasColourOtherThan(const QTextBlock &block, const QColor &surfaceColour)
{
    for (QTextBlock::iterator it = block.begin(); !it.atEnd(); ++it) {
        const QTextFragment fragment = it.fragment();
        if (!fragment.isValid())
            continue;

        const QBrush foreground = fragment.charFormat().foreground();
        if (foreground.style() == Qt::NoBrush)
            continue;

        if (foreground.color() != surfaceColour)
            return true;
    }
    return false;
}

QTextBlock appendBlock(QTextCursor &cursor, const QTextBlockFormat &format, const QString &text)
{
    cursor.insertBlock(format, QTextCharFormat());
    cursor.insertText(text);
    return cursor.block();
}

}   // namespace

TEST(UT_MarkdownRenderer, Render_NullDocument_IsIgnored)
{
    MarkdownRenderer renderer;

    EXPECT_NO_THROW(renderer.render(nullptr, QStringLiteral("# heading"), QString(), renderTheme()));
}

TEST(UT_MarkdownRenderer, Render_ConfiguresMarginFontAndWrapMode)
{
    const MarkdownTheme theme = renderTheme();
    QTextDocument document;
    MarkdownRenderer renderer;

    renderer.render(&document, QStringLiteral("# Heading\n\nbody text"), QString(), theme);

    EXPECT_EQ(document.documentMargin(), theme.documentMargin);
    EXPECT_EQ(document.defaultFont().family(), theme.textFontFamily);
    EXPECT_DOUBLE_EQ(document.defaultFont().pointSizeF(), theme.baseFontPointSize);
    EXPECT_EQ(document.defaultTextOption().wrapMode(), QTextOption::WrapAtWordBoundaryOrAnywhere);
    EXPECT_TRUE(document.toPlainText().contains(QStringLiteral("Heading")));
    EXPECT_TRUE(document.toPlainText().contains(QStringLiteral("body text")));
}

TEST(UT_MarkdownRenderer, Render_EmptyMarkdown_ClearsPreviousContent)
{
    MarkdownRenderer renderer;
    QTextDocument document;
    document.setPlainText(QStringLiteral("previous content"));
    ASSERT_FALSE(document.isEmpty());

    renderer.render(&document, QString(), QString(), renderTheme());

    EXPECT_TRUE(document.isEmpty());
    EXPECT_TRUE(document.toPlainText().isEmpty());
}

TEST(UT_MarkdownRenderer, Render_Heading_UsesLargerBoldFontInThemeColour)
{
    const MarkdownTheme theme = renderTheme();
    QTextDocument document;
    MarkdownRenderer renderer;

    renderer.render(&document, QStringLiteral("# Section\n\nbody"), QString(), theme);

    const QTextBlock heading = findBlock(document, QStringLiteral("Section"));
    ASSERT_TRUE(heading.isValid());
    EXPECT_GT(heading.charFormat().fontPointSize(), theme.baseFontPointSize);
    EXPECT_GE(heading.charFormat().fontWeight(), int(QFont::Bold));
    EXPECT_EQ(heading.charFormat().foreground().color(), theme.headingColor);
}

TEST(UT_MarkdownRenderer, Render_FencedCode_HighlightsCodeButNotProse)
{
    const MarkdownTheme theme = renderTheme();
    QTextDocument document;
    MarkdownRenderer renderer;

    renderer.render(&document,
                    QStringLiteral("prose before\n\n```cpp\nint value = 0;\n```\n\nprose after\n"),
                    QString(),
                    theme);

    const QTextBlock code = findBlock(document, QStringLiteral("int value = 0;"));
    ASSERT_TRUE(code.isValid());
    EXPECT_TRUE(MarkdownRenderer::isCodeBlock(code, theme));
    EXPECT_TRUE(hasColourOtherThan(code, theme.codeTextColor));

    const QTextBlock before = findBlock(document, QStringLiteral("prose before"));
    ASSERT_TRUE(before.isValid());
    EXPECT_FALSE(MarkdownRenderer::isCodeBlock(before, theme));
    EXPECT_FALSE(hasColourOtherThan(before, QColor()));

    const QTextBlock after = findBlock(document, QStringLiteral("prose after"));
    ASSERT_TRUE(after.isValid());
    EXPECT_FALSE(MarkdownRenderer::isCodeBlock(after, theme));
    EXPECT_FALSE(hasColourOtherThan(after, QColor()));
}

TEST(UT_MarkdownRenderer, Render_CodeLanguagesFollowFenceOrder_UnknownFirst)
{
    const MarkdownTheme theme = renderTheme();
    QTextDocument document;
    MarkdownRenderer renderer;

    renderer.render(&document,
                    QStringLiteral("```\nplain block\n```\n\nseparator paragraph\n\n"
                                   "```cpp\nint value = 0;\n```\n"),
                    QString(),
                    theme);

    const QTextBlock plain = findBlock(document, QStringLiteral("plain block"));
    ASSERT_TRUE(plain.isValid());
    EXPECT_TRUE(MarkdownRenderer::isCodeBlock(plain, theme));
    EXPECT_FALSE(hasColourOtherThan(plain, theme.codeTextColor));

    const QTextBlock cpp = findBlock(document, QStringLiteral("int value = 0;"));
    ASSERT_TRUE(cpp.isValid());
    EXPECT_TRUE(MarkdownRenderer::isCodeBlock(cpp, theme));
    EXPECT_TRUE(hasColourOtherThan(cpp, theme.codeTextColor));
}

TEST(UT_MarkdownRenderer, Render_CodeLanguagesFollowFenceOrder_KnownFirst)
{
    const MarkdownTheme theme = renderTheme();
    QTextDocument document;
    MarkdownRenderer renderer;

    renderer.render(&document,
                    QStringLiteral("```cpp\nint value = 0;\n```\n\nseparator paragraph\n\n"
                                   "```\nplain block\n```\n"),
                    QString(),
                    theme);

    const QTextBlock cpp = findBlock(document, QStringLiteral("int value = 0;"));
    ASSERT_TRUE(cpp.isValid());
    EXPECT_TRUE(hasColourOtherThan(cpp, theme.codeTextColor));

    const QTextBlock plain = findBlock(document, QStringLiteral("plain block"));
    ASSERT_TRUE(plain.isValid());
    EXPECT_FALSE(hasColourOtherThan(plain, theme.codeTextColor));
}

TEST(UT_MarkdownRenderer, Render_UnknownFenceLanguage_LeavesCodeUnhighlighted)
{
    const MarkdownTheme theme = renderTheme();
    QTextDocument document;
    MarkdownRenderer renderer;

    renderer.render(&document, QStringLiteral("```no-such-language\nint value = 0;\n```\n"),
                    QString(), theme);

    const QTextBlock code = findBlock(document, QStringLiteral("int value = 0;"));
    ASSERT_TRUE(code.isValid());
    EXPECT_TRUE(MarkdownRenderer::isCodeBlock(code, theme));
    EXPECT_FALSE(hasColourOtherThan(code, theme.codeTextColor));
}

TEST(UT_MarkdownRenderer, IsCodeBlock_BlockFormatBackground_IsRecognised)
{
    const MarkdownTheme theme = renderTheme();
    QTextDocument document;
    QTextCursor cursor(&document);
    cursor.setBlockFormat(QTextBlockFormat());
    QTextBlockFormat codeFormat;
    codeFormat.setBackground(QBrush(theme.codeBackground));
    const QTextBlock block = appendBlock(cursor, codeFormat, QStringLiteral("code"));

    EXPECT_TRUE(MarkdownRenderer::isCodeBlock(block, theme));
}

TEST(UT_MarkdownRenderer, IsCodeBlock_CharFormatBackground_IsRecognised)
{
    const MarkdownTheme theme = renderTheme();
    QTextDocument document;
    QTextCursor cursor(&document);

    // QTextCursor::insertText() only sets the fragment format; the block char
    // format (what QTextBlock::charFormat() returns) is set separately, which
    // is exactly the second path isCodeBlock() has to recognise.
    QTextCharFormat charFormat;
    charFormat.setBackground(QBrush(theme.codeBackground));
    cursor.setBlockCharFormat(charFormat);
    cursor.insertText(QStringLiteral("code"));

    const QTextBlock block = document.firstBlock();
    EXPECT_EQ(block.blockFormat().background().style(), Qt::NoBrush);
    EXPECT_TRUE(MarkdownRenderer::isCodeBlock(block, theme));
}

TEST(UT_MarkdownRenderer, IsCodeBlock_CharFormatBackground_MatchesByRgb)
{
    const MarkdownTheme theme = renderTheme();
    QTextDocument document;
    QTextCursor cursor(&document);

    // Imported HTML carries 8 bit colours, so a theme colour with 16 bit
    // channels must still be recognised.
    QColor imported = theme.codeBackground;
    imported.setRgb(theme.codeBackground.rgb());

    QTextCharFormat charFormat;
    charFormat.setBackground(QBrush(imported));
    cursor.setBlockCharFormat(charFormat);
    cursor.insertText(QStringLiteral("code"));

    EXPECT_TRUE(MarkdownRenderer::isCodeBlock(document.firstBlock(), theme));
}

TEST(UT_MarkdownRenderer, IsCodeBlock_ForeignBackground_IsRejected)
{
    const MarkdownTheme theme = renderTheme();
    QTextDocument document;
    QTextCursor cursor(&document);

    QTextBlockFormat blockFormat;
    blockFormat.setBackground(QBrush(QColor(0x01, 0x02, 0x03)));
    const QTextBlock block = appendBlock(cursor, blockFormat, QStringLiteral("code"));

    EXPECT_FALSE(MarkdownRenderer::isCodeBlock(block, theme));
}

TEST(UT_MarkdownRenderer, IsCodeBlock_NoBackground_IsRejected)
{
    const MarkdownTheme theme = renderTheme();
    QTextDocument document;
    document.setPlainText(QStringLiteral("plain"));

    EXPECT_FALSE(MarkdownRenderer::isCodeBlock(document.firstBlock(), theme));
}

TEST(UT_MarkdownRenderer, HighlightCodeBlocks_EmptyLanguageList_ChangesNothing)
{
    const MarkdownTheme theme = renderTheme();
    QTextDocument document;
    QTextCursor cursor(&document);
    QTextBlockFormat codeFormat;
    codeFormat.setBackground(QBrush(theme.codeBackground));
    const QTextBlock block = appendBlock(cursor, codeFormat, QStringLiteral("int value = 0;"));

    MarkdownRenderer::highlightCodeBlocks(&document, QStringList(), theme);

    EXPECT_FALSE(hasColourOtherThan(block, theme.codeTextColor));
}

TEST(UT_MarkdownRenderer, HighlightCodeBlocks_FewerLanguagesThanRuns_HighlightsOnlyFirstRun)
{
    const MarkdownTheme theme = renderTheme();
    QTextDocument document;
    QTextCursor cursor(&document);

    QTextBlockFormat codeFormat;
    codeFormat.setBackground(QBrush(theme.codeBackground));

    const QTextBlock first = appendBlock(cursor, codeFormat, QStringLiteral("int value = 0;"));
    const QTextBlock separator = appendBlock(cursor, QTextBlockFormat(), QStringLiteral("separator"));
    const QTextBlock second = appendBlock(cursor, codeFormat, QStringLiteral("int other = 1;"));

    MarkdownRenderer::highlightCodeBlocks(&document, QStringList { QStringLiteral("cpp") }, theme);

    EXPECT_TRUE(hasColourOtherThan(first, theme.codeTextColor));
    EXPECT_FALSE(hasColourOtherThan(separator, QColor()));
    EXPECT_FALSE(hasColourOtherThan(second, theme.codeTextColor));
}
