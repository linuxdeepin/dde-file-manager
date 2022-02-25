/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: lixiang<lixianga@uniontech.com>
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

#include "preview_plugin_global.h"
#include "dfm-base/interfaces/abstractbasepreview.h"

#include <QObject>
#include <QPointer>

PREVIEW_BEGIN_NAMESPACE
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
PREVIEW_END_NAMESPACE
#endif   // PDFPREVIEW_H
