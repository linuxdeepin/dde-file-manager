#include "pdfpreviewplugin.h"

DFM_USE_NAMESPACE
DFMFilePreview *PDFPreviewPlugin::create(const QString &key)
{
    Q_UNUSED(key)

    return new PDFPreview();
}
