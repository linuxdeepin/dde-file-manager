// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MARKDOWNRENDERER_H
#define MARKDOWNRENDERER_H

#include "markdowntheme.h"
#include "preview_plugin_global.h"

#include <QString>
#include <QStringList>

class QTextBlock;
class QTextDocument;

namespace plugin_filepreview {

/*!
 * \brief Renders markdown into a QTextDocument.
 *
 * Two passes: md4c turns the source into the HTML subset understood by
 * QTextHtmlParser, then fenced code blocks are re-coloured with
 * KSyntaxHighlighting because Qt's rich text engine has no notion of a
 * language inside a <pre> element.
 */
class MarkdownRenderer
{
public:
    void render(QTextDocument *document, const QString &markdown, const QString &basePath,
                const MarkdownTheme &theme) const;

private:
    static bool isCodeBlock(const QTextBlock &block, const MarkdownTheme &theme);
    static void highlightCodeBlocks(QTextDocument *document, const QStringList &languages,
                                    const MarkdownTheme &theme);
};

}   // namespace plugin_filepreview

#endif   // MARKDOWNRENDERER_H
