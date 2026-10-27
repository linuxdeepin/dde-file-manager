// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "markdownhtmlbuilder.h"

#include <QDir>
#include <QImageReader>
#include <QUrl>

namespace plugin_filepreview {
namespace {

QString escapeHtml(const QString &text)
{
    QString escaped = text;
    escaped.replace(QLatin1Char('&'), QLatin1String("&amp;"));
    escaped.replace(QLatin1Char('<'), QLatin1String("&lt;"));
    escaped.replace(QLatin1Char('>'), QLatin1String("&gt;"));
    return escaped;
}

QString escapeAttribute(const QString &text)
{
    QString escaped = escapeHtml(text);
    escaped.replace(QLatin1Char('"'), QLatin1String("&quot;"));
    return escaped;
}

QString numberOf(qreal value, int precision = 1)
{
    return QString::number(value, 'f', precision);
}

QString attributeText(const MD_ATTRIBUTE &attribute)
{
    if (!attribute.text || attribute.size == 0)
        return QString();

    return QString::fromUtf8(attribute.text, static_cast<int>(attribute.size));
}

}   // namespace

MarkdownHtmlBuilder::MarkdownHtmlBuilder(const MarkdownTheme &theme, const QString &basePath)
    : m_theme(theme),
      m_basePath(basePath)
{
}

QString MarkdownHtmlBuilder::build(const QString &markdown)
{
    m_html.clear();
    m_codeLanguages.clear();
    m_inCodeBlock = false;
    m_captureImageAlt = false;

    if (markdown.isEmpty())
        return QString();

    MD_PARSER parser = {};
    parser.flags = MD_DIALECT_GITHUB;
    parser.enter_block = &MarkdownHtmlBuilder::enterBlock;
    parser.leave_block = &MarkdownHtmlBuilder::leaveBlock;
    parser.enter_span = &MarkdownHtmlBuilder::enterSpan;
    parser.leave_span = &MarkdownHtmlBuilder::leaveSpan;
    parser.text = &MarkdownHtmlBuilder::appendText;

    // md4c works on bytes and only reports slices of the input, so the UTF-8
    // buffer has to stay alive for the whole call.
    const QByteArray utf8 = markdown.toUtf8();
    md_parse(utf8.constData(), static_cast<MD_SIZE>(utf8.size()), &parser, this);

    return m_html;
}

int MarkdownHtmlBuilder::enterBlock(MD_BLOCKTYPE type, void *detail, void *userdata)
{
    static_cast<MarkdownHtmlBuilder *>(userdata)->onEnterBlock(type, detail);
    return 0;
}

int MarkdownHtmlBuilder::leaveBlock(MD_BLOCKTYPE type, void *detail, void *userdata)
{
    static_cast<MarkdownHtmlBuilder *>(userdata)->onLeaveBlock(type, detail);
    return 0;
}

int MarkdownHtmlBuilder::enterSpan(MD_SPANTYPE type, void *detail, void *userdata)
{
    static_cast<MarkdownHtmlBuilder *>(userdata)->onEnterSpan(type, detail);
    return 0;
}

int MarkdownHtmlBuilder::leaveSpan(MD_SPANTYPE type, void *detail, void *userdata)
{
    static_cast<MarkdownHtmlBuilder *>(userdata)->onLeaveSpan(type, detail);
    return 0;
}

int MarkdownHtmlBuilder::appendText(MD_TEXTTYPE type, const MD_CHAR *text, MD_SIZE size, void *userdata)
{
    static_cast<MarkdownHtmlBuilder *>(userdata)->onText(type, QString::fromUtf8(text, static_cast<int>(size)));
    return 0;
}

void MarkdownHtmlBuilder::onEnterBlock(MD_BLOCKTYPE type, void *detail)
{
    switch (type) {
    case MD_BLOCK_DOC:
        break;
    case MD_BLOCK_QUOTE:
        // Qt's <blockquote> only indents and offers no way to draw the left
        // bar. A one cell table with a solid left border renders the bar, keeps
        // block margins in px, and lets nested content (lists, fences, deeper
        // quotes) live inside the cell.
        m_html += QStringLiteral("<table width=\"100%\" cellspacing=\"0\" cellpadding=\"0\" "
                                 "style=\"margin-left:12px; margin-top:2px; margin-bottom:8px;\">"
                                 "<tr><td style=\"border-left:3px solid %1; padding-left:12px; color:%2;\">")
                          .arg(m_theme.quoteBarColor.name(), m_theme.quoteTextColor.name());
        break;
    case MD_BLOCK_UL:
        m_html += QStringLiteral("<ul style=\"margin-left:6px; margin-top:2px; margin-bottom:8px;\">");
        break;
    case MD_BLOCK_OL: {
        auto *olDetail = static_cast<MD_BLOCK_OL_DETAIL *>(detail);
        QString tag = QStringLiteral("<ol style=\"margin-left:10px; margin-top:2px; margin-bottom:8px;\"");
        if (olDetail && olDetail->start > 1)
            tag += QStringLiteral(" start=\"%1\"").arg(olDetail->start);
        m_html += tag + QLatin1Char('>');
        break;
    }
    case MD_BLOCK_LI: {
        auto *liDetail = static_cast<MD_BLOCK_LI_DETAIL *>(detail);
        m_html += QStringLiteral("<li style=\"margin-bottom:2px;\">");

        if (liDetail && liDetail->is_task) {
            const bool checked = liDetail->task_mark == QLatin1Char('x') || liDetail->task_mark == QLatin1Char('X');
            m_html += checked ? QStringLiteral("☑ ") : QStringLiteral("☐ ");
        }
        break;
    }
    case MD_BLOCK_HR:
        // <hr> always draws in the text colour and ignores margin styles; a
        // single cell with a top border takes the theme colour instead.
        m_html += QStringLiteral("<table width=\"100%\" cellspacing=\"0\" cellpadding=\"0\" "
                                 "style=\"margin-top:12px; margin-bottom:12px;\">"
                                 "<tr><td style=\"border-top:1px solid %1;\"></td></tr></table>")
                          .arg(m_theme.quoteBarColor.name());
        break;
    case MD_BLOCK_H: {
        auto *hDetail = static_cast<MD_BLOCK_H_DETAIL *>(detail);
        // QTextHtmlParser pins h1..h6 to its own size table and ignores
        // font-size, so headings are emitted as sized bold paragraphs.
        const int level = hDetail ? qBound(1, static_cast<int>(hDetail->level), 6) : 1;
        m_html += QStringLiteral("<p style=\"font-size:%1pt; font-weight:bold; color:%2; "
                                 "margin-top:%3px; margin-bottom:%4px;\">")
                          .arg(numberOf(m_theme.headingPointSize(level)))
                          .arg(m_theme.headingColor.name())
                          .arg(m_theme.headingTopMargin(level))
                          .arg(m_theme.headingBottomMargin(level));
        break;
    }
    case MD_BLOCK_CODE: {
        auto *codeDetail = static_cast<MD_BLOCK_CODE_DETAIL *>(detail);
        m_codeLanguages.append(codeDetail ? attributeText(codeDetail->lang) : QString());
        m_inCodeBlock = true;

        m_html += QStringLiteral("<pre style=\"background-color:%1; color:%2; "
                                 "font-family:'%3'; font-size:%4pt; "
                                 "margin-top:6px; margin-bottom:12px;\">")
                          .arg(m_theme.codeBackground.name(),
                               m_theme.codeTextColor.name(),
                               m_theme.monoFontFamily,
                               numberOf(m_theme.baseFontPointSize - 0.5));
        m_codeContentStart = m_html.size();
        break;
    }
    case MD_BLOCK_HTML:
        break;
    case MD_BLOCK_P:
        m_html += QStringLiteral("<p style=\"margin-top:0px; margin-bottom:10px;\">");
        break;
    case MD_BLOCK_TABLE:
        m_html += QStringLiteral("<table border=\"1\" cellspacing=\"0\" cellpadding=\"6\" width=\"100%\" "
                                 "style=\"border-collapse:collapse; margin-top:6px; margin-bottom:12px;\">");
        break;
    case MD_BLOCK_THEAD:
    case MD_BLOCK_TBODY:
        // Rows are emitted directly inside <table>; Qt's importer is happy with
        // that and support for the grouping tags is inconsistent.
        break;
    case MD_BLOCK_TR:
        m_html += QStringLiteral("<tr>");
        break;
    case MD_BLOCK_TH:
    case MD_BLOCK_TD: {
        auto *cellDetail = static_cast<MD_BLOCK_TD_DETAIL *>(detail);
        const MD_ALIGN align = cellDetail ? cellDetail->align : MD_ALIGN_DEFAULT;
        const char *alignment = "";
        switch (align) {
        case MD_ALIGN_CENTER:
            alignment = " align=\"center\"";
            break;
        case MD_ALIGN_RIGHT:
            alignment = " align=\"right\"";
            break;
        case MD_ALIGN_LEFT:
        case MD_ALIGN_DEFAULT:
            break;
        }

        if (type == MD_BLOCK_TH) {
            m_html += QStringLiteral("<th") + QString::fromLatin1(alignment)
                    + QStringLiteral(" bgcolor=\"") + m_theme.tableHeaderBackground.name()
                    + QStringLiteral("\" style=\"font-weight:bold;\">");
        } else {
            m_html += QStringLiteral("<td") + QString::fromLatin1(alignment) + QLatin1Char('>');
        }
        break;
    }
    }
}

void MarkdownHtmlBuilder::onLeaveBlock(MD_BLOCKTYPE type, void *detail)
{
    Q_UNUSED(detail)

    switch (type) {
    case MD_BLOCK_DOC:
    case MD_BLOCK_HTML:
        break;
    case MD_BLOCK_QUOTE:
        m_html += QStringLiteral("</td></tr></table>");
        break;
    case MD_BLOCK_UL:
        m_html += QStringLiteral("</ul>");
        break;
    case MD_BLOCK_OL:
        m_html += QStringLiteral("</ol>");
        break;
    case MD_BLOCK_LI:
        m_html += QStringLiteral("</li>");
        break;
    case MD_BLOCK_HR:
        break;
    case MD_BLOCK_H:
        m_html += QStringLiteral("</p>");
        break;
    case MD_BLOCK_CODE:
        // An empty fence gets a zero width space so it still becomes a block:
        // the renderer pairs code blocks with fence languages in document
        // order, and a missing block would shift every later language.
        if (m_html.size() == m_codeContentStart)
            m_html += QStringLiteral("\u200B");
        m_html += QStringLiteral("</pre>");
        m_inCodeBlock = false;
        break;
    case MD_BLOCK_P:
        m_html += QStringLiteral("</p>");
        break;
    case MD_BLOCK_TABLE:
        m_html += QStringLiteral("</table>");
        break;
    case MD_BLOCK_THEAD:
    case MD_BLOCK_TBODY:
        break;
    case MD_BLOCK_TR:
        m_html += QStringLiteral("</tr>");
        break;
    case MD_BLOCK_TH:
        m_html += QStringLiteral("</th>");
        break;
    case MD_BLOCK_TD:
        m_html += QStringLiteral("</td>");
        break;
    }
}

void MarkdownHtmlBuilder::onEnterSpan(MD_SPANTYPE type, void *detail)
{
    if (m_captureImageAlt)
        return;

    switch (type) {
    case MD_SPAN_EM:
        m_html += QStringLiteral("<i>");
        break;
    case MD_SPAN_STRONG:
        m_html += QStringLiteral("<b>");
        break;
    case MD_SPAN_A: {
        auto *linkDetail = static_cast<MD_SPAN_A_DETAIL *>(detail);
        const QString href = linkDetail ? attributeText(linkDetail->href) : QString();
        m_html += QStringLiteral("<a href=\"%1\" style=\"color:%2;\">")
                          .arg(escapeAttribute(href), m_theme.linkColor.name());
        break;
    }
    case MD_SPAN_IMG: {
        auto *imageDetail = static_cast<MD_SPAN_IMG_DETAIL *>(detail);
        m_captureImageAlt = true;
        m_imageSource = imageDetail ? attributeText(imageDetail->src) : QString();
        m_imageAlt.clear();
        break;
    }
    case MD_SPAN_CODE:
        m_html += QStringLiteral("<span style=\"font-family:'%1'; background-color:%2; color:%3;\">")
                          .arg(m_theme.monoFontFamily,
                               m_theme.inlineCodeBackground.name(),
                               m_theme.codeTextColor.name());
        break;
    case MD_SPAN_DEL:
        m_html += QStringLiteral("<s>");
        break;
    case MD_SPAN_LATEXMATH:
        m_html += QStringLiteral("<span style=\"color:%1; font-style:italic;\">").arg(m_theme.mutedColor.name());
        break;
    case MD_SPAN_LATEXMATH_DISPLAY:
        m_html += QStringLiteral("<span style=\"color:%1; font-style:italic; margin-left:12px; margin-right:12px;\">")
                          .arg(m_theme.mutedColor.name());
        break;
    case MD_SPAN_WIKILINK: {
        auto *wikiDetail = static_cast<MD_SPAN_WIKILINK_DETAIL *>(detail);
        const QString target = wikiDetail ? attributeText(wikiDetail->target) : QString();
        m_html += QStringLiteral("<a href=\"%1\">").arg(escapeAttribute(target));
        break;
    }
    case MD_SPAN_U:
        m_html += QStringLiteral("<u>");
        break;
    }
}

void MarkdownHtmlBuilder::onLeaveSpan(MD_SPANTYPE type, void *detail)
{
    Q_UNUSED(detail)

    if (type == MD_SPAN_IMG) {
        if (m_captureImageAlt) {
            m_captureImageAlt = false;
            m_html += imageTag(m_imageSource, m_imageAlt);
            m_imageSource.clear();
            m_imageAlt.clear();
        }
        return;
    }

    if (m_captureImageAlt)
        return;

    switch (type) {
    case MD_SPAN_EM:
        m_html += QStringLiteral("</i>");
        break;
    case MD_SPAN_STRONG:
        m_html += QStringLiteral("</b>");
        break;
    case MD_SPAN_A:
        m_html += QStringLiteral("</a>");
        break;
    case MD_SPAN_IMG:
        break;
    case MD_SPAN_CODE:
        m_html += QStringLiteral("</span>");
        break;
    case MD_SPAN_DEL:
        m_html += QStringLiteral("</s>");
        break;
    case MD_SPAN_LATEXMATH:
        m_html += QStringLiteral("</span>");
        break;
    case MD_SPAN_LATEXMATH_DISPLAY:
        m_html += QStringLiteral("</span>");
        break;
    case MD_SPAN_WIKILINK:
        m_html += QStringLiteral("</a>");
        break;
    case MD_SPAN_U:
        m_html += QStringLiteral("</u>");
        break;
    }
}

void MarkdownHtmlBuilder::onText(MD_TEXTTYPE type, const QString &text)
{
    if (m_captureImageAlt) {
        m_imageAlt += text;
        return;
    }

    switch (type) {
    case MD_TEXT_NORMAL:
    case MD_TEXT_CODE:
    case MD_TEXT_HTML:
    case MD_TEXT_LATEXMATH:
        m_html += escapeHtml(text);
        break;
    case MD_TEXT_NULLCHAR:
        break;
    case MD_TEXT_BR:
        m_html += QStringLiteral("<br />");
        break;
    case MD_TEXT_SOFTBR:
        // Outside of code blocks a soft break is a space; inside one the
        // newline is part of the text and reaches MD_TEXT_CODE instead.
        m_html += m_inCodeBlock ? QStringLiteral("\n") : QStringLiteral(" ");
        break;
    case MD_TEXT_ENTITY:
        m_html += text;
        break;
    }
}

QString MarkdownHtmlBuilder::resolveLocalPath(const QString &src) const
{
    const QUrl url(src);
    if (url.isLocalFile())
        return url.toLocalFile();

    if (!url.scheme().isEmpty())
        return QString();

    if (m_basePath.isEmpty())
        return src;

    return QDir(m_basePath).absoluteFilePath(src);
}

QString MarkdownHtmlBuilder::imageTag(const QString &src, const QString &alt) const
{
    if (src.isEmpty())
        return escapeHtml(alt);

    const QUrl url(src);
    if (!url.scheme().isEmpty() && url.scheme() != QLatin1String("file")) {
        // Remote images are never downloaded by the preview, so keep the
        // reference visible instead of showing a broken placeholder.
        return QStringLiteral("<a href=\"%1\">%2</a>")
                .arg(escapeAttribute(src), escapeHtml(alt.isEmpty() ? src : alt));
    }

    const QString path = resolveLocalPath(src);
    if (path.isEmpty())
        return escapeHtml(alt.isEmpty() ? src : alt);

    QImageReader reader(path);
    QSize size = reader.size();
    if (size.isEmpty()) {
        // Either not an image or unreadable: fall back to the alt text.
        return QStringLiteral("<i>%1</i>").arg(escapeHtml(alt.isEmpty() ? src : alt));
    }

    if (size.width() > m_theme.contentWidth) {
        size.setHeight(qMax(1, qRound(size.height() * qreal(m_theme.contentWidth) / size.width())));
        size.setWidth(m_theme.contentWidth);
    }

    return QStringLiteral("<img src=\"%1\" width=\"%2\" height=\"%3\" />")
            .arg(escapeAttribute(path),
                 QString::number(size.width()),
                 QString::number(size.height()));
}

}   // namespace plugin_filepreview
