/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             lixiang<lixianga@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

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
