// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PDFPREVIEW_H
#define PDFPREVIEW_H

#include <QObject>
#include <QPointer>

#include "dfmfilepreview.h"
#include "durl.h"

class PdfWidget;

class PDFPreview : public DFM_NAMESPACE::DFMFilePreview
{
    Q_OBJECT

public:
    explicit PDFPreview(QObject *parent = nullptr);
    ~PDFPreview() override;

    bool setFileUrl(const DUrl &url) override;
    DUrl fileUrl() const override;

    QWidget *contentWidget() const override;

    QString title() const override;
    bool showStatusBarSeparator() const override;

private:
    DUrl m_url;
    QString m_title;

    QPointer<PdfWidget> m_pdfWidget;
};

#endif // PDFPREVIEWPLUGIN_H
