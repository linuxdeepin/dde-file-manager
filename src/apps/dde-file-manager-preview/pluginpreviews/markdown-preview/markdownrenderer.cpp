// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "markdownrenderer.h"

#include "markdowncodehighlighter.h"
#include "markdownhtmlbuilder.h"

#include <QBrush>
#include <QFont>
#include <QTextBlock>
#include <QTextDocument>
#include <QTextOption>
#include <QVector>

namespace plugin_filepreview {
namespace {

QFont bodyFont(const MarkdownTheme &theme)
{
    QFont font(theme.textFontFamily);
    font.setPointSizeF(theme.baseFontPointSize);
    return font;
}

// Theme colours are derived with lighter()/darker() and keep 16 bit channels,
// while the generated markup carries them as 8 bit hex values. Qt parses those
// back into a different 16 bit value, so comparing the QColor objects directly
// never matches a theme built from the application palette. Compare the 8 bit
// value the markup actually carries.
bool sameColor(const QBrush &brush, const QColor &expected)
{
    return brush.style() != Qt::NoBrush && brush.color().rgb() == expected.rgb();
}

}   // namespace

void MarkdownRenderer::render(QTextDocument *document, const QString &markdown, const QString &basePath,
                              const MarkdownTheme &theme) const
{
    if (!document)
        return;

    document->setDocumentMargin(theme.documentMargin);
    document->setDefaultFont(bodyFont(theme));

    QTextOption textOption = document->defaultTextOption();
    textOption.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    document->setDefaultTextOption(textOption);

    MarkdownHtmlBuilder builder(theme, basePath);
    document->setHtml(builder.build(markdown));

    highlightCodeBlocks(document, builder.codeLanguages(), theme);
}

bool MarkdownRenderer::isCodeBlock(const QTextBlock &block, const MarkdownTheme &theme)
{
    // The <pre> background is the only marker Qt carries through to the
    // imported document. Qt has stored it on the block and on the block's char
    // format across versions, so both are checked. If a platform style drops
    // the colour entirely the block simply keeps its monospace font and stays
    // unhighlighted instead of being coloured by mistake.
    if (sameColor(block.blockFormat().background(), theme.codeBackground))
        return true;

    return sameColor(block.charFormat().background(), theme.codeBackground);
}

void MarkdownRenderer::highlightCodeBlocks(QTextDocument *document, const QStringList &languages,
                                           const MarkdownTheme &theme)
{
    if (languages.isEmpty())
        return;

    MarkdownCodeHighlighter highlighter(theme);

    int languageIndex = 0;
    QTextBlock block = document->begin();

    while (block.isValid()) {
        if (!isCodeBlock(block, theme)) {
            block = block.next();
            continue;
        }

        QVector<QTextBlock> run;
        while (block.isValid() && isCodeBlock(block, theme)) {
            run.append(block);
            block = block.next();
        }

        highlighter.highlightRun(run, languageIndex < languages.size()
                                          ? languages.at(languageIndex)
                                          : QString());
        ++languageIndex;
    }
}

}   // namespace plugin_filepreview
