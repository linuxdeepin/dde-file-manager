// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "markdownhtmlbuilder.h"
#include "markdowntheme.h"

#include <gtest/gtest.h>

#include <QColor>
#include <QDir>
#include <QFile>
#include <QImage>
#include <QStringList>
#include <QTemporaryDir>
#include <QUrl>

using namespace plugin_filepreview;

namespace {

// Deterministic palette so the generated HTML can be asserted literally.
MarkdownTheme testTheme()
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
    theme.textFontFamily = QStringLiteral("TestSans");
    theme.monoFontFamily = QStringLiteral("TestMono");
    theme.baseFontPointSize = 10.0;
    theme.documentMargin = 24;
    theme.contentWidth = 100;
    return theme;
}

QString build(const QString &markdown, QStringList *languages = nullptr,
              const QString &basePath = QString())
{
    const MarkdownTheme theme = testTheme();
    MarkdownHtmlBuilder builder(theme, basePath);
    const QString html = builder.build(markdown);
    if (languages)
        *languages = builder.codeLanguages();
    return html;
}

QString writeImage(const QDir &dir, const QString &name, int width, int height)
{
    QImage image(width, height, QImage::Format_ARGB32);
    image.fill(Qt::red);
    const QString path = dir.absoluteFilePath(name);
    image.save(path, "PNG");
    return path;
}

}   // namespace

TEST(UT_MarkdownHtmlBuilder, EmptyInput_ReturnsEmptyHtmlAndNoLanguages)
{
    QStringList languages;
    const QString html = build(QString(), &languages);

    EXPECT_TRUE(html.isEmpty());
    EXPECT_TRUE(languages.isEmpty());
}

TEST(UT_MarkdownHtmlBuilder, WhitespaceOnlyInput_ProducesNoBlocks)
{
    QStringList languages;
    const QString html = build(QStringLiteral("\n\n   \n"), &languages);

    EXPECT_TRUE(html.isEmpty());
    EXPECT_TRUE(languages.isEmpty());
}

TEST(UT_MarkdownHtmlBuilder, Headings_AllLevels_EmitSizedBoldParagraphs)
{
    const qreal sizes[] = { 19.0, 15.0, 12.8, 11.2, 10.0, 9.4 };
    const int tops[] = { 6, 18, 16, 12, 12, 12 };
    const int bottoms[] = { 10, 8, 6, 6, 6, 6 };

    for (int level = 1; level <= 6; ++level) {
        const QString markdown = QString(level, QLatin1Char('#')) + QStringLiteral(" Heading %1").arg(level);
        const QString html = build(markdown);

        const QString expected = QStringLiteral("<p style=\"font-size:%1pt; font-weight:bold; "
                                                "color:#223344; margin-top:%2px; margin-bottom:%3px;\">"
                                                "Heading %4</p>")
                                         .arg(QString::number(sizes[level - 1], 'f', 1))
                                         .arg(tops[level - 1])
                                         .arg(bottoms[level - 1])
                                         .arg(level);
        EXPECT_TRUE(html.contains(expected)) << "level " << level << " html=" << html.toStdString();
    }
}

TEST(UT_MarkdownHtmlBuilder, Paragraph_EmitsToplessBottomMargin)
{
    const QString html = build(QStringLiteral("plain paragraph"));

    EXPECT_EQ(html, QStringLiteral("<p style=\"margin-top:0px; margin-bottom:10px;\">plain paragraph</p>"));
}

TEST(UT_MarkdownHtmlBuilder, UnorderedList_EmitsMarginLeftOnListAndItems)
{
    const QString html = build(QStringLiteral("- one\n- two\n"));

    EXPECT_TRUE(html.contains(QStringLiteral(
            "<ul style=\"margin-left:6px; margin-top:2px; margin-bottom:8px;\">")));
    EXPECT_TRUE(html.contains(QStringLiteral("<li style=\"margin-bottom:2px;\">one</li>")));
    EXPECT_TRUE(html.contains(QStringLiteral("<li style=\"margin-bottom:2px;\">two</li>")));
    EXPECT_TRUE(html.endsWith(QStringLiteral("</ul>")));
}

TEST(UT_MarkdownHtmlBuilder, OrderedList_DefaultStart_OmitsStartAttribute)
{
    const QString html = build(QStringLiteral("1. one\n2. two\n"));

    EXPECT_TRUE(html.startsWith(QStringLiteral(
            "<ol style=\"margin-left:10px; margin-top:2px; margin-bottom:8px;\">")));
    EXPECT_FALSE(html.contains(QStringLiteral("start=")));
}

TEST(UT_MarkdownHtmlBuilder, OrderedList_NonDefaultStart_EmitsStartAttribute)
{
    const QString html = build(QStringLiteral("3. three\n4. four\n"));

    EXPECT_TRUE(html.startsWith(QStringLiteral(
            "<ol style=\"margin-left:10px; margin-top:2px; margin-bottom:8px;\" start=\"3\">")));
    EXPECT_TRUE(html.contains(QStringLiteral("<li style=\"margin-bottom:2px;\">three</li>")));
}

TEST(UT_MarkdownHtmlBuilder, TaskList_CheckedAndUnchecked_UseBoxGlyphs)
{
    const QString html = build(QStringLiteral("- [x] done\n- [ ] todo\n"));

    EXPECT_TRUE(html.contains(QStringLiteral("\u2611 done</li>")));   // ☑
    EXPECT_TRUE(html.contains(QStringLiteral("\u2610 todo</li>")));   // ☐
}

TEST(UT_MarkdownHtmlBuilder, Blockquote_WrapsContentInSingleCellTableWithLeftBar)
{
    const QString html = build(QStringLiteral("> quoted text"));

    EXPECT_TRUE(html.startsWith(QStringLiteral(
            "<table width=\"100%\" cellspacing=\"0\" cellpadding=\"0\" "
            "style=\"margin-left:12px; margin-top:2px; margin-bottom:8px;\">"
            "<tr><td style=\"border-left:3px solid #99aabb; padding-left:12px; color:#8899aa;\">")));
    EXPECT_TRUE(html.contains(QStringLiteral("quoted text")));
    EXPECT_TRUE(html.endsWith(QStringLiteral("</td></tr></table>")));
}

TEST(UT_MarkdownHtmlBuilder, HorizontalRule_UsesTableWithThemedTopBorder)
{
    const QString html = build(QStringLiteral("above\n\n---\n\nbelow"));

    EXPECT_TRUE(html.contains(QStringLiteral(
            "<table width=\"100%\" cellspacing=\"0\" cellpadding=\"0\" "
            "style=\"margin-top:12px; margin-bottom:12px;\">"
            "<tr><td style=\"border-top:1px solid #99aabb;\"></td></tr></table>")));
    EXPECT_TRUE(html.contains(QStringLiteral("above")));
    EXPECT_TRUE(html.contains(QStringLiteral("below")));
}

TEST(UT_MarkdownHtmlBuilder, FencedCode_RecordsLanguageAndEmitsThemedPre)
{
    QStringList languages;
    const QString html = build(QStringLiteral("```cpp\nint x = 0;\n```\n"), &languages);

    EXPECT_EQ(languages, QStringList { QStringLiteral("cpp") });
    EXPECT_TRUE(html.startsWith(QStringLiteral(
            "<pre style=\"background-color:#667788; color:#556677; font-family:'TestMono'; "
            "font-size:9.5pt; margin-top:6px; margin-bottom:12px;\">")));
    EXPECT_TRUE(html.contains(QStringLiteral("int x = 0;")));
    EXPECT_TRUE(html.endsWith(QStringLiteral("</pre>")));
}

TEST(UT_MarkdownHtmlBuilder, FencedCode_InfoStringWithAttributes_LanguageIsFirstWord)
{
    QStringList languages;
    build(QStringLiteral("```cpp title=\"demo\"\nint x = 0;\n```\n"), &languages);

    EXPECT_EQ(languages, QStringList { QStringLiteral("cpp") });
}

TEST(UT_MarkdownHtmlBuilder, IndentedCodeBlock_HasNoLanguage)
{
    QStringList languages;
    const QString html = build(QStringLiteral("    indented code\n"), &languages);

    EXPECT_EQ(languages, QStringList { QString() });
    EXPECT_TRUE(html.startsWith(QStringLiteral("<pre ")));
    EXPECT_TRUE(html.contains(QStringLiteral("indented code")));
}

TEST(UT_MarkdownHtmlBuilder, EmptyFence_EmitsZeroWidthSpaceSoBlockSurvives)
{
    QStringList languages;
    const QString html = build(QStringLiteral("```\n```\n"), &languages);

    EXPECT_EQ(languages, QStringList { QString() });
    EXPECT_TRUE(html.contains(QStringLiteral("\u200B</pre>")));
}

TEST(UT_MarkdownHtmlBuilder, MultipleFences_LanguagesStayInDocumentOrder)
{
    QStringList languages;
    build(QStringLiteral("```cpp\nint x = 0;\n```\n\n```python\nprint(1)\n```\n"), &languages);

    EXPECT_EQ(languages, (QStringList { QStringLiteral("cpp"), QStringLiteral("python") }));
}

TEST(UT_MarkdownHtmlBuilder, InlineCode_UsesMonoFontAndInlineBackground)
{
    const QString html = build(QStringLiteral("call `value` now"));

    EXPECT_TRUE(html.contains(QStringLiteral(
            "<span style=\"font-family:'TestMono'; background-color:#778899; color:#556677;\">"
            "value</span>")));
}

TEST(UT_MarkdownHtmlBuilder, EmphasisStrongAndStrike_EmitMatchingTags)
{
    const QString html = build(QStringLiteral("**bold** *italic* ~~gone~~"));

    EXPECT_TRUE(html.contains(QStringLiteral("<b>bold</b>")));
    EXPECT_TRUE(html.contains(QStringLiteral("<i>italic</i>")));
    EXPECT_TRUE(html.contains(QStringLiteral("<s>gone</s>")));
}

TEST(UT_MarkdownHtmlBuilder, Link_EmitsThemedAnchorAndEscapesHref)
{
    const QString html = build(QStringLiteral("[text](https://example.com/?a=1&b=2)"));

    EXPECT_TRUE(html.contains(QStringLiteral(
            "<a href=\"https://example.com/?a=1&amp;b=2\" style=\"color:#445566;\">text</a>")));
}

TEST(UT_MarkdownHtmlBuilder, PlainText_EscapesHtmlMetacharacters)
{
    const QString html = build(QStringLiteral("5 < 6 & 7 > 4"));

    EXPECT_TRUE(html.contains(QStringLiteral("5 &lt; 6 &amp; 7 &gt; 4")));
}

TEST(UT_MarkdownHtmlBuilder, RawHtml_IsEscapedInsteadOfExecuted)
{
    const QString inlineHtml = build(QStringLiteral("before <span>inner</span> after"));
    EXPECT_TRUE(inlineHtml.contains(QStringLiteral("&lt;span&gt;inner&lt;/span&gt;")));

    const QString blockHtml = build(QStringLiteral("<div>\nraw block\n</div>\n"));
    EXPECT_TRUE(blockHtml.contains(QStringLiteral("&lt;div&gt;")));
    EXPECT_TRUE(blockHtml.contains(QStringLiteral("raw block")));
}

TEST(UT_MarkdownHtmlBuilder, Entity_IsPassedThroughVerbatim)
{
    const QString html = build(QStringLiteral("AT&amp;T"));

    EXPECT_TRUE(html.contains(QStringLiteral("AT&amp;T")));
    EXPECT_FALSE(html.contains(QStringLiteral("&amp;amp;")));
}

TEST(UT_MarkdownHtmlBuilder, LineBreaks_HardBecomesBrSoftBecomesSpace)
{
    const QString hard = build(QStringLiteral("first  \nsecond"));
    EXPECT_TRUE(hard.contains(QStringLiteral("first<br />second")));

    const QString soft = build(QStringLiteral("third\nfourth"));
    EXPECT_TRUE(soft.contains(QStringLiteral("third fourth")));
}

TEST(UT_MarkdownHtmlBuilder, NullCharacter_IsDropped)
{
    const QString markdown = QStringLiteral("a") + QChar(0) + QStringLiteral("b");
    const QString html = build(markdown);

    EXPECT_FALSE(html.contains(QChar(0)));
    EXPECT_TRUE(html.contains(QStringLiteral("ab")));
}

TEST(UT_MarkdownHtmlBuilder, Table_EmitsBordersHeaderBackgroundAndAlignment)
{
    const QString html = build(QStringLiteral("| Left | Center | Right |\n"
                                              "|:-----|:------:|------:|\n"
                                              "| a    | b      | c     |\n"));

    EXPECT_TRUE(html.contains(QStringLiteral(
            "<table border=\"1\" cellspacing=\"0\" cellpadding=\"6\" width=\"100%\" "
            "style=\"border-collapse:collapse; margin-top:6px; margin-bottom:12px;\">")));
    EXPECT_TRUE(html.contains(QStringLiteral("<th bgcolor=\"#aabbcc\" style=\"font-weight:bold;\">Left</th>")));
    EXPECT_TRUE(html.contains(QStringLiteral(
            "<th align=\"center\" bgcolor=\"#aabbcc\" style=\"font-weight:bold;\">Center</th>")));
    EXPECT_TRUE(html.contains(QStringLiteral(
            "<th align=\"right\" bgcolor=\"#aabbcc\" style=\"font-weight:bold;\">Right</th>")));
    EXPECT_TRUE(html.contains(QStringLiteral("<td>a</td>")));
    EXPECT_TRUE(html.contains(QStringLiteral("<td align=\"center\">b</td>")));
    EXPECT_TRUE(html.contains(QStringLiteral("<td align=\"right\">c</td>")));
}

TEST(UT_MarkdownHtmlBuilder, LocalImage_ResolvesAgainstBasePathAndDownscales)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    const QString path = writeImage(QDir(dir.path()), QStringLiteral("wide.png"), 200, 80);
    ASSERT_TRUE(QFile::exists(path));

    const QString html = build(QStringLiteral("![pic](wide.png)"), nullptr, dir.path());

    const QString expected = QStringLiteral("<img src=\"%1\" width=\"100\" height=\"40\" />")
                                     .arg(path);
    EXPECT_TRUE(html.contains(expected)) << html.toStdString();
}

TEST(UT_MarkdownHtmlBuilder, LocalImage_SmallerThanContentWidth_KeepsOriginalSize)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    const QString path = writeImage(QDir(dir.path()), QStringLiteral("small.png"), 40, 20);

    const QString html = build(QStringLiteral("![pic](small.png)"), nullptr, dir.path());

    EXPECT_TRUE(html.contains(QStringLiteral("<img src=\"%1\" width=\"40\" height=\"20\" />")
                                      .arg(path)));
}

TEST(UT_MarkdownHtmlBuilder, FileUrlImage_IsConvertedToLocalPath)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    const QString path = writeImage(QDir(dir.path()), QStringLiteral("url.png"), 40, 20);

    const QString markdown = QStringLiteral("![pic](%1)").arg(QUrl::fromLocalFile(path).toString());
    const QString html = build(markdown);

    EXPECT_TRUE(html.contains(QStringLiteral("<img src=\"%1\" width=\"40\" height=\"20\" />")
                                      .arg(path)));
}

TEST(UT_MarkdownHtmlBuilder, MissingImage_FallsBackToAltText)
{
    const QString html = build(QStringLiteral("![missing](ut-missing-image-42.png)"));

    EXPECT_TRUE(html.contains(QStringLiteral("<i>missing</i>")));
    EXPECT_FALSE(html.contains(QStringLiteral("<img")));
}

TEST(UT_MarkdownHtmlBuilder, UnreadableImage_FallsBackToAltText)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    const QString path = QDir(dir.path()).absoluteFilePath(QStringLiteral("not-an-image.png"));
    QFile file(path);
    ASSERT_TRUE(file.open(QIODevice::WriteOnly));
    file.write("this is not a png");
    file.close();

    const QString html = build(QStringLiteral("![broken](not-an-image.png)"), nullptr, dir.path());

    EXPECT_TRUE(html.contains(QStringLiteral("<i>broken</i>")));
    EXPECT_FALSE(html.contains(QStringLiteral("<img")));
}

TEST(UT_MarkdownHtmlBuilder, RemoteImage_BecomesLinkInsteadOfBrokenImage)
{
    const QString html = build(QStringLiteral("![logo](https://example.com/logo.png)"));

    EXPECT_TRUE(html.contains(QStringLiteral(
            "<a href=\"https://example.com/logo.png\">logo</a>")));
    EXPECT_FALSE(html.contains(QStringLiteral("<img")));
}

TEST(UT_MarkdownHtmlBuilder, EmptyImageDestination_ProducesNoImageTag)
{
    const QString html = build(QStringLiteral("![]()"));

    EXPECT_FALSE(html.contains(QStringLiteral("<img")));
}

TEST(UT_MarkdownHtmlBuilder, ImageAltText_FormattingIsFlattenedToPlainText)
{
    const QString html = build(QStringLiteral("![**bold** alt](ut-missing-image-42.png)"));

    EXPECT_TRUE(html.contains(QStringLiteral("<i>bold alt</i>")));
}

TEST(UT_MarkdownHtmlBuilder, Build_Reused_ClearsPreviousHtmlAndLanguages)
{
    MarkdownTheme theme = testTheme();
    MarkdownHtmlBuilder builder(theme, QString());

    const QString first = builder.build(QStringLiteral("```cpp\nint x = 0;\n```\n"));
    ASSERT_FALSE(first.isEmpty());
    ASSERT_EQ(builder.codeLanguages(), QStringList { QStringLiteral("cpp") });

    const QString second = builder.build(QStringLiteral("plain text"));

    EXPECT_FALSE(second.contains(QStringLiteral("int x")));
    EXPECT_FALSE(second.contains(QStringLiteral("<pre")));
    EXPECT_TRUE(second.contains(QStringLiteral("plain text")));
    EXPECT_TRUE(builder.codeLanguages().isEmpty());
}

TEST(UT_MarkdownHtmlBuilder, UnicodeText_IsPreservedAndInlineMarkupStillApplies)
{
    const QString html = build(QStringLiteral("\u4f60\u597d **\u4e16\u754c**"));

    EXPECT_TRUE(html.contains(QStringLiteral("\u4f60\u597d")));
    EXPECT_TRUE(html.contains(QStringLiteral("<b>\u4e16\u754c</b>")));
}

// The GFM dialect md4c is configured with does not enable LaTeX, wiki links or
// underline, so those SpanType branches never run through build(). They are
// still part of the builder contract and are exercised directly here.
TEST(UT_MarkdownHtmlBuilder, NonGfmSpanTypes_StillEmitWellFormedMarkup)
{
    MarkdownTheme theme = testTheme();
    MarkdownHtmlBuilder builder(theme, QString());

    builder.onEnterSpan(MD_SPAN_LATEXMATH, nullptr);
    builder.onText(MD_TEXT_LATEXMATH, QStringLiteral("x^2"));
    builder.onLeaveSpan(MD_SPAN_LATEXMATH, nullptr);

    builder.onEnterSpan(MD_SPAN_LATEXMATH_DISPLAY, nullptr);
    builder.onLeaveSpan(MD_SPAN_LATEXMATH_DISPLAY, nullptr);

    builder.onEnterSpan(MD_SPAN_WIKILINK, nullptr);
    builder.onText(MD_TEXT_NORMAL, QStringLiteral("page"));
    builder.onLeaveSpan(MD_SPAN_WIKILINK, nullptr);

    builder.onEnterSpan(MD_SPAN_U, nullptr);
    builder.onText(MD_TEXT_NORMAL, QStringLiteral("underlined"));
    builder.onLeaveSpan(MD_SPAN_U, nullptr);

    EXPECT_TRUE(builder.m_html.contains(QStringLiteral("font-style:italic")));
    EXPECT_TRUE(builder.m_html.contains(QStringLiteral("x^2")));
    EXPECT_TRUE(builder.m_html.contains(QStringLiteral(">page</a>")));
    EXPECT_TRUE(builder.m_html.contains(QStringLiteral("<u>underlined</u>")));
}
