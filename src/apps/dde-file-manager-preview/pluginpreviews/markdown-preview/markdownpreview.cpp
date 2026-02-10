// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "markdownpreview.h"
#include "markdownbrowser.h"
#include "markdowncontextwidget.h"

#include <dfm-base/interfaces/fileinfo.h>

#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QDebug>

using namespace plugin_filepreview;
DFMBASE_USE_NAMESPACE

// 5MB maximum file size
static constexpr qint64 kMaxReadSize { 1024 * 1024 * 5 };

MarkdownPreview::MarkdownPreview(QObject *parent)
    : AbstractBasePreview(parent)
{
    fmInfo() << "Markdown preview: MarkdownPreview instance created";
}

MarkdownPreview::~MarkdownPreview()
{
    fmInfo() << "Markdown preview: MarkdownPreview instance destroyed";
    if (m_markdownBrowser) {
        // Set parent to nullptr first to prevent recursive destruction issues
        m_markdownBrowser->setParent(nullptr);
        m_markdownBrowser->deleteLater();
        m_markdownBrowser = nullptr;
    }
}

bool MarkdownPreview::setFileUrl(const QUrl &url)
{
    fmInfo() << "Markdown preview: setting file URL:" << url;

    if (m_selectUrl == url) {
        fmDebug() << "Markdown preview: URL unchanged, skipping:" << url;
        return true;
    }

    if (!url.isLocalFile()) {
        fmWarning() << "Markdown preview: URL is not a local file:" << url;
        return false;
    }

    const QString filePath = url.toLocalFile();
    if (!QFileInfo::exists(filePath)) {
        fmWarning() << "Markdown preview: file does not exist:" << filePath;
        return false;
    }

    QFileInfo fileInfo(filePath);
    qint64 fileSize = fileInfo.size();

    if (fileSize <= 0) {
        fmWarning() << "Markdown preview: file is empty or cannot determine size:" << filePath;
        return false;
    }

    fmDebug() << "Markdown preview: file size:" << fileSize << "bytes, will read up to:" << kMaxReadSize << "bytes";

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        fmWarning() << "Markdown preview: failed to open file:" << filePath << "error:" << file.errorString();
        return false;
    }

    // Limit read size to 5MB
    qint64 readSize = qMin(fileSize, kMaxReadSize);
    if (fileSize > kMaxReadSize) {
        fmDebug() << "Markdown preview: file size exceeds limit, truncating to:" << kMaxReadSize << "bytes";
    }

    // Read file content
    QByteArray data = file.read(readSize);
    file.close();

    if (data.isEmpty()) {
        fmWarning() << "Markdown preview: failed to read file content:" << filePath;
        return false;
    }

    // Convert to QString
    const QString markdownContent = QString::fromUtf8(data);

    // Create browser widget if not exists
    if (!m_markdownBrowser) {
        fmDebug() << "Markdown preview: creating new MarkdownContextWidget";
        m_markdownBrowser = new MarkdownContextWidget;
    }

    // Set markdown content with base path for relative images
    QString basePath = fileInfo.absolutePath();
    m_markdownBrowser->markdownBrowser()->setMarkdownContent(markdownContent, basePath);

    m_selectUrl = url;
    m_titleStr = fileInfo.fileName();

    fmInfo() << "Markdown preview: file loaded successfully:" << filePath << "title:" << m_titleStr;
    Q_EMIT titleChanged();

    return true;
}

QUrl MarkdownPreview::fileUrl() const
{
    return m_selectUrl;
}

QWidget *MarkdownPreview::contentWidget() const
{
    return m_markdownBrowser;
}

QString MarkdownPreview::title() const
{
    return m_titleStr;
}

bool MarkdownPreview::showStatusBarSeparator() const
{
    return false;
}
