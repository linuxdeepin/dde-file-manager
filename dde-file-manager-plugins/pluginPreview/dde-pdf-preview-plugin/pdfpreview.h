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
