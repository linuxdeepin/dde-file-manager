// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MARKDOWNHTMLBUILDER_H
#define MARKDOWNHTMLBUILDER_H

#include "markdowntheme.h"
#include "preview_plugin_global.h"

#include <md4c.h>

#include <QString>
#include <QStringList>

namespace plugin_filepreview {

/*!
 * \brief Converts markdown into an HTML fragment for QTextDocument.
 *
 * QTextDocument understands only a small subset of HTML/CSS, which is why
 * QTextDocument::setMarkdown() cannot express headings, spacing, code
 * surfaces or table styling. This builder parses with md4c (the same parser
 * Qt uses internally, so the dialect matches) and emits the tags and *inline
 * style attributes* that QTextHtmlParser does honour.
 */
class MarkdownHtmlBuilder
{
public:
    MarkdownHtmlBuilder(const MarkdownTheme &theme, const QString &basePath);

    QString build(const QString &markdown);

    //! Fence language of every code block, in document order.
    QStringList codeLanguages() const { return m_codeLanguages; }

private:
    static int enterBlock(MD_BLOCKTYPE type, void *detail, void *userdata);
    static int leaveBlock(MD_BLOCKTYPE type, void *detail, void *userdata);
    static int enterSpan(MD_SPANTYPE type, void *detail, void *userdata);
    static int leaveSpan(MD_SPANTYPE type, void *detail, void *userdata);
    static int appendText(MD_TEXTTYPE type, const MD_CHAR *text, MD_SIZE size, void *userdata);

    void onEnterBlock(MD_BLOCKTYPE type, void *detail);
    void onLeaveBlock(MD_BLOCKTYPE type, void *detail);
    void onEnterSpan(MD_SPANTYPE type, void *detail);
    void onLeaveSpan(MD_SPANTYPE type, void *detail);
    void onText(MD_TEXTTYPE type, const QString &text);

    QString imageTag(const QString &src, const QString &alt) const;
    QString resolveLocalPath(const QString &src) const;

    MarkdownTheme m_theme;
    QString m_basePath;
    QString m_html;
    QStringList m_codeLanguages;

    bool m_inCodeBlock { false };
    bool m_captureImageAlt { false };
    qsizetype m_codeContentStart { 0 };
    QString m_imageSource;
    QString m_imageAlt;
};

}   // namespace plugin_filepreview

#endif   // MARKDOWNHTMLBUILDER_H
