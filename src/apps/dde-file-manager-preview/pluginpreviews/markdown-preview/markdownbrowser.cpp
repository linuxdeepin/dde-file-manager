// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "markdownbrowser.h"

#include "markdowntheme.h"
#include "markdownrenderer.h"

#include <DGuiApplicationHelper>

#include <QDebug>
#include <QDir>
#include <QScrollBar>
#include <QStringList>
#include <QTextCursor>
#include <QTextDocument>
#include <QUrl>

using namespace plugin_filepreview;
DGUI_USE_NAMESPACE

MarkdownBrowser::MarkdownBrowser(QWidget *parent)
    : QTextBrowser(parent)
{
    setReadOnly(true);
    setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard | Qt::LinksAccessibleByMouse);
    setOpenExternalLinks(true);
    setLineWrapMode(QTextBrowser::WidgetWidth);
    setFixedSize(800, 500);
    setFrameStyle(QFrame::NoFrame);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
            this, &MarkdownBrowser::onThemeTypeChanged);
}

MarkdownBrowser::~MarkdownBrowser()
{
}

void MarkdownBrowser::setMarkdownContent(const QString &markdown, const QString &basePath)
{
    fmDebug() << "Markdown preview: setting markdown content, base path:" << basePath;

    m_markdown = markdown;
    m_basePath = basePath;

    renderContent(true);

    fmInfo() << "Markdown preview: content set successfully";
}

void MarkdownBrowser::onThemeTypeChanged()
{
    // Colours live inside the generated HTML, so the document has to be built
    // again to follow a light/dark switch.
    if (!m_markdown.isEmpty())
        renderContent(false);
}

void MarkdownBrowser::renderContent(bool resetScrollPosition)
{
    const int scrollPosition = verticalScrollBar()->value();

    const MarkdownTheme theme = MarkdownTheme::fromApplication();
    MarkdownRenderer renderer;
    renderer.render(document(), m_markdown, m_basePath, theme);

    // Applied after the import: setHtml() replaces the document contents and
    // the location has to survive it.
    if (!m_basePath.isEmpty()) {
        if (QDir(m_basePath).exists()) {
            setSearchPaths(QStringList() << m_basePath);
            // The trailing slash keeps relative targets inside the folder
            // instead of resolving them against its parent.
            document()->setBaseUrl(QUrl::fromLocalFile(m_basePath + QLatin1Char('/')));
            document()->setMetaInformation(QTextDocument::DocumentUrl,
                                           QUrl::fromLocalFile(m_basePath).toString());
        } else {
            fmWarning() << "Markdown preview: base path does not exist:" << m_basePath;
        }
    }

    if (resetScrollPosition) {
        verticalScrollBar()->setValue(0);
        moveCursor(QTextCursor::Start, QTextCursor::MoveAnchor);
    } else {
        // A theme switch should not throw the reader back to the top.
        verticalScrollBar()->setValue(scrollPosition);
    }
}
