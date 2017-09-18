/*
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

#ifndef PDFPREVIEWPLUGIN_H
#define PDFPREVIEWPLUGIN_H

#include <QObject>
#include <QPointer>

#include "dfmfilepreview.h"
#include "durl.h"

class PdfWidget;

DFM_BEGIN_NAMESPACE

class PDFPreview : public DFMFilePreview
{
    Q_OBJECT

public:
    PDFPreview(QObject *parent = 0);
    ~PDFPreview();

    bool setFileUrl(const DUrl &url) Q_DECL_OVERRIDE;

    QWidget *contentWidget() const Q_DECL_OVERRIDE;

    QString title() const Q_DECL_OVERRIDE;
    bool showStatusBarSeparator() const Q_DECL_OVERRIDE;

private:
    DUrl m_url;
    QString m_title;

    QPointer<PdfWidget> m_pdfWidget;
};

DFM_END_NAMESPACE

#endif // PDFPREVIEWPLUGIN_H
