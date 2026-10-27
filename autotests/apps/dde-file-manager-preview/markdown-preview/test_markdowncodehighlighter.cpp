// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "markdowncodehighlighter.h"
#include "markdowntheme.h"

#include <KSyntaxHighlighting/Definition>
#include <KSyntaxHighlighting/Format>
#include <KSyntaxHighlighting/Repository>
#include <KSyntaxHighlighting/Theme>

#include <gtest/gtest.h>

#include <QTextBlock>
#include <QTextDocument>
#include <QTextFragment>
#include <QVector>

#include <string>

using namespace plugin_filepreview;

namespace {

bool hasForegroundFormat(const QTextBlock &block)
{
    for (QTextBlock::iterator it = block.begin(); !it.atEnd(); ++it) {
        const QTextFragment fragment = it.fragment();
        if (fragment.isValid() && fragment.charFormat().foreground().style() != Qt::NoBrush)
            return true;
    }
    return false;
}

QVector<QTextBlock> blocksOf(QTextDocument &document)
{
    QVector<QTextBlock> blocks;
    for (QTextBlock block = document.begin(); block.isValid(); block = block.next())
        blocks.append(block);
    return blocks;
}

MarkdownTheme lightTheme()
{
    MarkdownTheme theme;
    theme.dark = false;
    return theme;
}

MarkdownTheme darkTheme()
{
    MarkdownTheme theme;
    theme.dark = true;
    return theme;
}

}   // namespace

TEST(UT_MarkdownCodeHighlighter, Repository_RepeatedCalls_ReturnsSameInstance)
{
    EXPECT_EQ(&MarkdownCodeHighlighter::repository(),
              &MarkdownCodeHighlighter::repository());
}

TEST(UT_MarkdownCodeHighlighter, DefinitionFor_CommonFenceAliases_ResolveToDefinitions)
{
    struct Case
    {
        const char *fence;
        const char *canonicalName;
    };

    const Case cases[] = {
        { "c++", "C++" },
        { "cpp", "C++" },
        { "cxx", "C++" },
        { "cc", "C++" },
        { "h", "C++" },
        { "hpp", "C++" },
        { "c", "C" },
        { "js", "JavaScript" },
        { "javascript", "JavaScript" },
        { "mjs", "JavaScript" },
        { "jsx", "JavaScript" },
        { "ts", "TypeScript" },
        { "typescript", "TypeScript" },
        { "tsx", "TypeScript" },
        { "py", "Python" },
        { "python", "Python" },
        { "python3", "Python" },
        { "sh", "Bash" },
        { "bash", "Bash" },
        { "zsh", "Bash" },
        { "rs", "Rust" },
        { "yml", "YAML" },
        { "yaml", "YAML" },
        { "json", "JSON" },
        { "html", "HTML" },
        { "xml", "XML" },
        { "css", "CSS" },
        { "md", "Markdown" },
        { "markdown", "Markdown" },
        { "go", "Go" },
        { "java", "Java" },
        { "kt", "Kotlin" },
        { "kotlin", "Kotlin" },
        { "qml", "QML" },
        { "cmake", "CMake" },
        { "diff", "Diff" },
        { "sql", "SQL" },
        // The repository names these two differently from the alias table
        // ("PHP (HTML)" and "INI Files"), so they only resolve through the
        // name / file name fallbacks. They must still end up valid.
        { "php", "" },
        { "ini", "" },
        { "rb", "Ruby" },
        { "cs", "C#" },
        { "swift", "Swift" },
        { "lua", "Lua" },
        { "toml", "TOML" },
        { "dockerfile", "Dockerfile" },
        { "makefile", "Makefile" },
        { "tex", "LaTeX" },
    };

    for (const Case &item : cases) {
        const KSyntaxHighlighting::Definition definition =
                MarkdownCodeHighlighter::definitionFor(QString::fromLatin1(item.fence));

        SCOPED_TRACE(std::string(item.fence) + " -> " + item.canonicalName);
        ASSERT_TRUE(definition.isValid());

        if (!item.canonicalName[0])
            continue;

        // The alias table must point at the definition the repository exposes
        // under its canonical name.
        const KSyntaxHighlighting::Definition reference =
                MarkdownCodeHighlighter::repository().definitionForName(
                        QString::fromLatin1(item.canonicalName));
        ASSERT_TRUE(reference.isValid());
        EXPECT_EQ(definition.name(), reference.name());
    }
}

TEST(UT_MarkdownCodeHighlighter, DefinitionFor_AliasesWithDifferentRepositoryNames_UseFallback)
{
    // Documents the fallback path: the alias table says "PHP"/"INI", which the
    // repository does not know, so the file name lookup has to rescue them.
    EXPECT_EQ(MarkdownCodeHighlighter::definitionFor(QStringLiteral("php")).name(),
              MarkdownCodeHighlighter::repository().definitionForFileName(
                      QStringLiteral("fence.php")).name());
    EXPECT_EQ(MarkdownCodeHighlighter::definitionFor(QStringLiteral("ini")).name(),
              MarkdownCodeHighlighter::repository().definitionForFileName(
                      QStringLiteral("fence.ini")).name());
}

TEST(UT_MarkdownCodeHighlighter, DefinitionFor_EveryAliasInTheTable_Resolves)
{
    // The alias table is a plain QHash, so an entry whose target the repository
    // does not know would silently disable highlighting for that fence. Cover
    // every source key so a stale name is caught here.
    const char *fences[] = {
        "c++", "cpp", "cxx", "cc", "h", "hpp", "c", "js", "javascript", "mjs",
        "jsx", "node", "ts", "typescript", "tsx", "py", "python", "python3",
        "sh", "bash", "shell", "zsh", "console", "rs", "rust", "yml", "yaml",
        "json", "jsonc", "html", "htm", "xml", "css", "scss", "less", "md",
        "markdown", "go", "golang", "java", "kt", "kotlin", "qml", "cmake",
        "diff", "patch", "sql", "php", "rb", "ruby", "pl", "perl", "cs",
        "csharp", "swift", "lua", "ini", "toml", "dockerfile", "docker",
        "makefile", "make", "tex", "latex"
    };

    for (const char *fence : fences) {
        const KSyntaxHighlighting::Definition definition =
                MarkdownCodeHighlighter::definitionFor(QString::fromLatin1(fence));
        SCOPED_TRACE(fence);
        EXPECT_TRUE(definition.isValid());
    }
}

TEST(UT_MarkdownCodeHighlighter, DefinitionFor_ExactNameAndCaseInsensitive_LookUp)
{
    const KSyntaxHighlighting::Definition exact =
            MarkdownCodeHighlighter::definitionFor(QStringLiteral("C++"));
    ASSERT_TRUE(exact.isValid());
    EXPECT_EQ(exact.name(), QStringLiteral("C++"));

    const KSyntaxHighlighting::Definition upper =
            MarkdownCodeHighlighter::definitionFor(QStringLiteral("PYTHON"));
    ASSERT_TRUE(upper.isValid());
    EXPECT_EQ(upper.name(), QStringLiteral("Python"));

    const KSyntaxHighlighting::Definition padded =
            MarkdownCodeHighlighter::definitionFor(QStringLiteral("  cpp  "));
    ASSERT_TRUE(padded.isValid());
    EXPECT_EQ(padded.name(), QStringLiteral("C++"));
}

TEST(UT_MarkdownCodeHighlighter, DefinitionFor_UnknownOrEmpty_IsInvalid)
{
    EXPECT_FALSE(MarkdownCodeHighlighter::definitionFor(QString()).isValid());
    EXPECT_FALSE(MarkdownCodeHighlighter::definitionFor(QStringLiteral("   ")).isValid());
    EXPECT_FALSE(MarkdownCodeHighlighter::definitionFor(
                         QStringLiteral("definitely-not-a-real-language"))
                         .isValid());
}

TEST(UT_MarkdownCodeHighlighter, Constructor_LightTheme_MatchesRepositoryDefault)
{
    MarkdownCodeHighlighter highlighter(lightTheme());

    const KSyntaxHighlighting::Theme expected =
            MarkdownCodeHighlighter::repository().defaultTheme(
                    KSyntaxHighlighting::Repository::LightTheme);

    EXPECT_TRUE(highlighter.theme().isValid());
    EXPECT_EQ(highlighter.theme().name(), expected.name());
}

TEST(UT_MarkdownCodeHighlighter, Constructor_DarkTheme_MatchesRepositoryDefault)
{
    MarkdownCodeHighlighter highlighter(darkTheme());

    const KSyntaxHighlighting::Theme expected =
            MarkdownCodeHighlighter::repository().defaultTheme(
                    KSyntaxHighlighting::Repository::DarkTheme);

    EXPECT_TRUE(highlighter.theme().isValid());
    EXPECT_EQ(highlighter.theme().name(), expected.name());
}

TEST(UT_MarkdownCodeHighlighter, HighlightRun_CppSource_AppliesKeywordColours)
{
    MarkdownCodeHighlighter highlighter(lightTheme());
    QTextDocument document;
    document.setPlainText(QStringLiteral("int value = 0;"));

    const QVector<QTextBlock> blocks = blocksOf(document);
    ASSERT_EQ(blocks.size(), 1);

    highlighter.highlightRun(blocks, QStringLiteral("cpp"));

    EXPECT_TRUE(hasForegroundFormat(blocks.first()));
    EXPECT_EQ(blocks.first().text(), QStringLiteral("int value = 0;"));
}

TEST(UT_MarkdownCodeHighlighter, HighlightRun_UnknownLanguage_LeavesTextUntouched)
{
    MarkdownCodeHighlighter highlighter(lightTheme());
    QTextDocument document;
    document.setPlainText(QStringLiteral("int value = 0;"));

    const QVector<QTextBlock> blocks = blocksOf(document);
    ASSERT_EQ(blocks.size(), 1);

    highlighter.highlightRun(blocks, QStringLiteral("no-such-language"));

    EXPECT_FALSE(hasForegroundFormat(blocks.first()));
}

TEST(UT_MarkdownCodeHighlighter, HighlightRun_EmptyLanguage_LeavesTextUntouched)
{
    MarkdownCodeHighlighter highlighter(lightTheme());
    QTextDocument document;
    document.setPlainText(QStringLiteral("plain code"));

    const QVector<QTextBlock> blocks = blocksOf(document);
    ASSERT_EQ(blocks.size(), 1);

    highlighter.highlightRun(blocks, QString());

    EXPECT_FALSE(hasForegroundFormat(blocks.first()));
}

TEST(UT_MarkdownCodeHighlighter, HighlightRun_EmptyBlockList_DoesNothing)
{
    MarkdownCodeHighlighter highlighter(lightTheme());

    EXPECT_NO_THROW(highlighter.highlightRun(QVector<QTextBlock>(), QStringLiteral("cpp")));
}

TEST(UT_MarkdownCodeHighlighter, HighlightRun_MultiLineRun_KeepsStateAcrossBlocks)
{
    MarkdownCodeHighlighter highlighter(lightTheme());
    QTextDocument document;
    document.setPlainText(QStringLiteral("def greet():\n    return 1"));

    const QVector<QTextBlock> blocks = blocksOf(document);
    ASSERT_EQ(blocks.size(), 2);

    highlighter.highlightRun(blocks, QStringLiteral("python"));

    EXPECT_TRUE(hasForegroundFormat(blocks.at(0)));
    EXPECT_TRUE(hasForegroundFormat(blocks.at(1)));
}

TEST(UT_MarkdownCodeHighlighter, HighlightRun_AliasAndUpperCase_BothHighlight)
{
    MarkdownCodeHighlighter highlighter(lightTheme());

    QTextDocument aliasDocument;
    aliasDocument.setPlainText(QStringLiteral("def greet():"));
    const QVector<QTextBlock> aliasBlocks = blocksOf(aliasDocument);
    ASSERT_EQ(aliasBlocks.size(), 1);
    highlighter.highlightRun(aliasBlocks, QStringLiteral("py"));
    EXPECT_TRUE(hasForegroundFormat(aliasBlocks.first()));

    QTextDocument upperDocument;
    upperDocument.setPlainText(QStringLiteral("def greet():"));
    const QVector<QTextBlock> upperBlocks = blocksOf(upperDocument);
    ASSERT_EQ(upperBlocks.size(), 1);
    highlighter.highlightRun(upperBlocks, QStringLiteral("PYTHON"));
    EXPECT_TRUE(hasForegroundFormat(upperBlocks.first()));
}

TEST(UT_MarkdownCodeHighlighter, HighlightRun_ReusedWithAnotherLanguage_DoesNotLeakState)
{
    MarkdownCodeHighlighter highlighter(lightTheme());

    QTextDocument first;
    first.setPlainText(QStringLiteral("int value = 0;"));
    const QVector<QTextBlock> firstBlocks = blocksOf(first);
    ASSERT_EQ(firstBlocks.size(), 1);
    highlighter.highlightRun(firstBlocks, QStringLiteral("cpp"));
    EXPECT_TRUE(hasForegroundFormat(firstBlocks.first()));

    QTextDocument second;
    second.setPlainText(QStringLiteral("plain words only"));
    const QVector<QTextBlock> secondBlocks = blocksOf(second);
    ASSERT_EQ(secondBlocks.size(), 1);
    highlighter.highlightRun(secondBlocks, QString());
    EXPECT_FALSE(hasForegroundFormat(secondBlocks.first()));
}

TEST(UT_MarkdownCodeHighlighter, ApplyFormat_InvalidArguments_DoNotFormatText)
{
    MarkdownCodeHighlighter highlighter(lightTheme());
    QTextDocument document;
    document.setPlainText(QStringLiteral("abc"));
    const QTextBlock block = document.firstBlock();

    const KSyntaxHighlighting::Format emptyFormat;

    // No block bound yet: the highlighter must not touch anything.
    highlighter.m_block = QTextBlock();
    highlighter.applyFormat(0, 1, emptyFormat);

    highlighter.m_block = block;
    highlighter.applyFormat(-1, 2, emptyFormat);
    highlighter.applyFormat(0, 0, emptyFormat);
    highlighter.applyFormat(2, -3, emptyFormat);

    EXPECT_FALSE(hasForegroundFormat(document.firstBlock()));
}
