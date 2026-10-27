// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "markdowncodehighlighter.h"

#include <KSyntaxHighlighting/Format>

#include <QHash>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QTextDocument>

namespace plugin_filepreview {
namespace {

// Fence info strings are GitHub flavoured, the definition names used by
// KSyntaxHighlighting are not. Map the common aliases once.
const QHash<QString, QString> &languageAliases()
{
    static const QHash<QString, QString> aliases {
        { QStringLiteral("c++"), QStringLiteral("C++") },
        { QStringLiteral("cpp"), QStringLiteral("C++") },
        { QStringLiteral("cxx"), QStringLiteral("C++") },
        { QStringLiteral("cc"), QStringLiteral("C++") },
        { QStringLiteral("h"), QStringLiteral("C++") },
        { QStringLiteral("hpp"), QStringLiteral("C++") },
        { QStringLiteral("c"), QStringLiteral("C") },
        { QStringLiteral("js"), QStringLiteral("JavaScript") },
        { QStringLiteral("javascript"), QStringLiteral("JavaScript") },
        { QStringLiteral("mjs"), QStringLiteral("JavaScript") },
        { QStringLiteral("jsx"), QStringLiteral("JavaScript") },
        { QStringLiteral("node"), QStringLiteral("JavaScript") },
        { QStringLiteral("ts"), QStringLiteral("TypeScript") },
        { QStringLiteral("typescript"), QStringLiteral("TypeScript") },
        { QStringLiteral("tsx"), QStringLiteral("TypeScript") },
        { QStringLiteral("py"), QStringLiteral("Python") },
        { QStringLiteral("python"), QStringLiteral("Python") },
        { QStringLiteral("python3"), QStringLiteral("Python") },
        { QStringLiteral("sh"), QStringLiteral("Bash") },
        { QStringLiteral("bash"), QStringLiteral("Bash") },
        { QStringLiteral("shell"), QStringLiteral("Bash") },
        { QStringLiteral("zsh"), QStringLiteral("Bash") },
        { QStringLiteral("console"), QStringLiteral("Bash") },
        { QStringLiteral("rs"), QStringLiteral("Rust") },
        { QStringLiteral("rust"), QStringLiteral("Rust") },
        { QStringLiteral("yml"), QStringLiteral("YAML") },
        { QStringLiteral("yaml"), QStringLiteral("YAML") },
        { QStringLiteral("json"), QStringLiteral("JSON") },
        { QStringLiteral("jsonc"), QStringLiteral("JSON") },
        { QStringLiteral("html"), QStringLiteral("HTML") },
        { QStringLiteral("htm"), QStringLiteral("HTML") },
        { QStringLiteral("xml"), QStringLiteral("XML") },
        { QStringLiteral("css"), QStringLiteral("CSS") },
        { QStringLiteral("scss"), QStringLiteral("SCSS") },
        { QStringLiteral("less"), QStringLiteral("LESS") },
        { QStringLiteral("md"), QStringLiteral("Markdown") },
        { QStringLiteral("markdown"), QStringLiteral("Markdown") },
        { QStringLiteral("go"), QStringLiteral("Go") },
        { QStringLiteral("golang"), QStringLiteral("Go") },
        { QStringLiteral("java"), QStringLiteral("Java") },
        { QStringLiteral("kt"), QStringLiteral("Kotlin") },
        { QStringLiteral("kotlin"), QStringLiteral("Kotlin") },
        { QStringLiteral("qml"), QStringLiteral("QML") },
        { QStringLiteral("cmake"), QStringLiteral("CMake") },
        { QStringLiteral("diff"), QStringLiteral("Diff") },
        { QStringLiteral("patch"), QStringLiteral("Diff") },
        { QStringLiteral("sql"), QStringLiteral("SQL") },
        { QStringLiteral("php"), QStringLiteral("PHP") },
        { QStringLiteral("rb"), QStringLiteral("Ruby") },
        { QStringLiteral("ruby"), QStringLiteral("Ruby") },
        { QStringLiteral("pl"), QStringLiteral("Perl") },
        { QStringLiteral("perl"), QStringLiteral("Perl") },
        { QStringLiteral("cs"), QStringLiteral("C#") },
        { QStringLiteral("csharp"), QStringLiteral("C#") },
        { QStringLiteral("swift"), QStringLiteral("Swift") },
        { QStringLiteral("lua"), QStringLiteral("Lua") },
        { QStringLiteral("ini"), QStringLiteral("INI") },
        { QStringLiteral("toml"), QStringLiteral("TOML") },
        { QStringLiteral("dockerfile"), QStringLiteral("Dockerfile") },
        { QStringLiteral("docker"), QStringLiteral("Dockerfile") },
        { QStringLiteral("makefile"), QStringLiteral("Makefile") },
        { QStringLiteral("make"), QStringLiteral("Makefile") },
        { QStringLiteral("tex"), QStringLiteral("LaTeX") },
        { QStringLiteral("latex"), QStringLiteral("LaTeX") },
    };
    return aliases;
}

}   // namespace

MarkdownCodeHighlighter::MarkdownCodeHighlighter(const MarkdownTheme &theme)
{
    setTheme(repository().defaultTheme(theme.dark ? KSyntaxHighlighting::Repository::DarkTheme
                                                  : KSyntaxHighlighting::Repository::LightTheme));
}

KSyntaxHighlighting::Repository &MarkdownCodeHighlighter::repository()
{
    // Building the repository scans the syntax definition directory, so keep a
    // single instance for the lifetime of the plugin.
    static KSyntaxHighlighting::Repository instance;
    return instance;
}

KSyntaxHighlighting::Definition MarkdownCodeHighlighter::definitionFor(const QString &language)
{
    const QString fence = language.trimmed();
    if (fence.isEmpty())
        return {};

    const QString lowered = fence.toLower();
    const QString alias = languageAliases().value(lowered, fence);

    KSyntaxHighlighting::Definition definition = repository().definitionForName(alias);
    if (!definition.isValid())
        definition = repository().definitionForName(fence);
    if (!definition.isValid())
        definition = repository().definitionForFileName(QStringLiteral("fence.") + lowered);

    return definition;
}

void MarkdownCodeHighlighter::highlightRun(const QVector<QTextBlock> &blocks, const QString &language)
{
    if (blocks.isEmpty())
        return;

    const KSyntaxHighlighting::Definition definition = definitionFor(language);
    if (!definition.isValid())
        return;

    setDefinition(definition);
    m_state = KSyntaxHighlighting::State();

    for (const QTextBlock &block : blocks) {
        m_block = block;
        m_state = highlightLine(block.text(), m_state);
    }

    m_block = QTextBlock();
}

void MarkdownCodeHighlighter::applyFormat(int offset, int length, const KSyntaxHighlighting::Format &format)
{
    if (!m_block.isValid() || offset < 0 || length <= 0)
        return;

    const KSyntaxHighlighting::Theme &syntaxTheme = theme();
    QTextCharFormat charFormat;

    if (!format.isDefaultTextStyle(syntaxTheme)) {
        const QColor foreground = format.textColor(syntaxTheme);
        if (foreground.isValid())
            charFormat.setForeground(foreground);
    }
    if (format.isBold(syntaxTheme))
        charFormat.setFontWeight(QFont::Bold);
    if (format.isItalic(syntaxTheme))
        charFormat.setFontItalic(true);
    if (format.isUnderline(syntaxTheme))
        charFormat.setFontUnderline(true);
    if (format.isStrikeThrough(syntaxTheme))
        charFormat.setFontStrikeOut(true);

    if (charFormat.isEmpty())
        return;

    QTextCursor cursor(m_block);
    const int blockPosition = m_block.position();
    cursor.setPosition(blockPosition + offset);
    cursor.setPosition(blockPosition + offset + length, QTextCursor::KeepAnchor);
    cursor.mergeCharFormat(charFormat);
}

}   // namespace plugin_filepreview
