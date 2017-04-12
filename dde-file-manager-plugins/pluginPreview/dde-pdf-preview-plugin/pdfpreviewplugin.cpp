#include "pdfpreviewplugin.h"

#include <QMimeType>
#include <QMimeDatabase>
#include <QUrl>
#include <QFileInfo>

#include "pdfwidget.h"

PDFPreviewPlugin::PDFPreviewPlugin(QObject *parent) :
    QObject(parent)
{
}

void PDFPreviewPlugin::init(const QString &uri)
{
    m_uri = uri;
}

QWidget *PDFPreviewPlugin::previewWidget()
{
    return new PdfWidget(m_uri);
}

QSize PDFPreviewPlugin::previewWidgetMinSize() const
{
    return QSize(400, 500);
}

bool PDFPreviewPlugin::canPreview() const
{
    QUrl url(m_uri);
    QFileInfo info(url.path());

    if(info.suffix() == "pdf"){
        return true;
    }

    QMimeDatabase md;
    QMimeType mimeType = md.mimeTypeForFile(url.path());
    if(mimeType.name() == "application/pdf"){
        return true;
    }

    return false;
}

QWidget *PDFPreviewPlugin::toolBarItem()
{
    return NULL;
}

QString PDFPreviewPlugin::pluginName() const
{
    return QString("dde-pdf-preview-plugin");
}

QIcon PDFPreviewPlugin::pluginLogo() const
{
    return QIcon();
}

QString PDFPreviewPlugin::pluginDescription() const
{
    return QString("Deepin offical pdf file preview plugin.");
}
