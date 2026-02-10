// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PDFPREVIEW_H
#define PDFPREVIEW_H

#include "preview_plugin_global.h"
#include <dfm-base/interfaces/abstractbasepreview.h>

#include <QObject>
#include <QPointer>

namespace plugin_filepreview {
class PdfWidget;

class PDFPreview : public DFMBASE_NAMESPACE::AbstractBasePreview
{
    Q_OBJECT
public:
    explicit PDFPreview(QObject *parent = nullptr);
    ~PDFPreview() override;

    bool setFileUrl(const QUrl &url) override;
    QUrl fileUrl() const override;

    QWidget *contentWidget() const override;

    QString title() const override;
    bool showStatusBarSeparator() const override;

    void initialize(QWidget *window, QWidget *statusBar) override;

private:
    QUrl selectFileUrl;
    QString pageTitle;

    QPointer<PdfWidget> pdfWidget;
};
}
#endif   // PDFPREVIEW_H
