// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MARKDOWNPREVIEW_H
#define MARKDOWNPREVIEW_H

#include "preview_plugin_global.h"

#include <dfm-base/interfaces/abstractbasepreview.h>

#include <QWidget>
#include <QPointer>
#include <QString>
#include <QUrl>

namespace plugin_filepreview {
class MarkdownContextWidget;
class MarkdownPreview : public DFMBASE_NAMESPACE::AbstractBasePreview
{
    Q_OBJECT

public:
    explicit MarkdownPreview(QObject *parent = nullptr);
    ~MarkdownPreview() override;

    bool setFileUrl(const QUrl &url) override;
    QUrl fileUrl() const override;

    QWidget *contentWidget() const override;

    QString title() const override;
    bool showStatusBarSeparator() const override;

private:
    QUrl m_selectUrl;
    QString m_titleStr;

    MarkdownContextWidget *m_markdownBrowser { nullptr };
};
}
#endif   // MARKDOWNPREVIEW_H
