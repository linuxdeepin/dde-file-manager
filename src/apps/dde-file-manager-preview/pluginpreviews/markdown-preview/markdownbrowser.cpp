// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "markdownbrowser.h"

#include <QTextDocument>
#include <QDebug>
#include <QDir>

using namespace plugin_filepreview;

MarkdownBrowser::MarkdownBrowser(QWidget *parent)
    : QTextBrowser(parent)
{
    setReadOnly(true);
    setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard | Qt::LinksAccessibleByMouse);
    setOpenExternalLinks(true);
    setLineWrapMode(QTextBrowser::WidgetWidth);
    setFixedSize(800, 500);
    setFrameStyle(QFrame::NoFrame);
}

MarkdownBrowser::~MarkdownBrowser()
{
}

void MarkdownBrowser::setMarkdownContent(const QString &markdown, const QString &basePath)
{
    fmDebug() << "Markdown preview: setting markdown content, base path:" << basePath;

    // Set base path for resolving relative image paths
    if (!basePath.isEmpty()) {
        QDir baseDir(basePath);
        if (baseDir.exists()) {
            document()->setMetaInformation(QTextDocument::DocumentUrl, QUrl::fromLocalFile(basePath).toString());
            setSearchPaths(QStringList() << basePath);
            fmDebug() << "Markdown preview: search paths set to:" << basePath;
        } else {
            fmWarning() << "Markdown preview: base path does not exist:" << basePath;
        }
    }

    // Set markdown content using QTextDocument
    document()->setMarkdown(markdown);

    // Move cursor to the start of the document to ensure top of content is visible
    moveCursor(QTextCursor::Start, QTextCursor::MoveAnchor);

    fmInfo() << "Markdown preview: content set successfully";
}
