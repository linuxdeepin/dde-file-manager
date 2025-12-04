// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MARKDOWNBROWSER_H
#define MARKDOWNBROWSER_H
#include "preview_plugin_global.h"

#include <QTextBrowser>

namespace plugin_filepreview {
class MarkdownBrowser : public QTextBrowser
{
    Q_OBJECT
public:
    explicit MarkdownBrowser(QWidget *parent = nullptr);

    virtual ~MarkdownBrowser() override;

    void setMarkdownContent(const QString &markdown, const QString &basePath);
};
}
#endif   // MARKDOWNBROWSER_H
