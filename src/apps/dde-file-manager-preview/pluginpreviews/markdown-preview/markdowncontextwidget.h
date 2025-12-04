// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MARKDOWNCONTEXTWIDGET_H
#define MARKDOWNCONTEXTWIDGET_H

#include <QWidget>

namespace plugin_filepreview {
class MarkdownBrowser;
class MarkdownContextWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MarkdownContextWidget(QWidget *parent = nullptr);
    plugin_filepreview::MarkdownBrowser *markdownBrowser() const;

private:
    plugin_filepreview::MarkdownBrowser *m_browserWidget { nullptr };
};
}
#endif // MARKDOWNCONTEXTWIDGET_H
