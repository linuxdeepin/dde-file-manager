// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MARKDOWNCODEHIGHLIGHTER_H
#define MARKDOWNCODEHIGHLIGHTER_H

#include "markdowntheme.h"
#include "preview_plugin_global.h"

#include <KSyntaxHighlighting/AbstractHighlighter>
#include <KSyntaxHighlighting/Definition>
#include <KSyntaxHighlighting/Repository>
#include <KSyntaxHighlighting/State>
#include <KSyntaxHighlighting/Theme>

#include <QTextBlock>
#include <QVector>

namespace plugin_filepreview {

/*!
 * \brief Highlights fenced code blocks of an already imported document.
 *
 * QSyntaxHighlighter works on a whole document and would need one instance per
 * fence language. Instead this highlighter is fed one run of blocks at a time,
 * so every fenced block can use the definition named by its own info string.
 */
class MarkdownCodeHighlighter : public KSyntaxHighlighting::AbstractHighlighter
{
public:
    explicit MarkdownCodeHighlighter(const MarkdownTheme &theme);

    void highlightRun(const QVector<QTextBlock> &blocks, const QString &language);

protected:
    void applyFormat(int offset, int length, const KSyntaxHighlighting::Format &format) override;

private:
    static KSyntaxHighlighting::Repository &repository();
    static KSyntaxHighlighting::Definition definitionFor(const QString &language);

    QTextBlock m_block;
    KSyntaxHighlighting::State m_state;
};

}   // namespace plugin_filepreview

#endif   // MARKDOWNCODEHIGHLIGHTER_H
