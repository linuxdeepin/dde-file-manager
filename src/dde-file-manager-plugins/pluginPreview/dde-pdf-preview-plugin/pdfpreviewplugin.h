#ifndef PDFPREVIEWPLUGIN_H
#define PDFPREVIEWPLUGIN_H
#include "dfmfilepreviewplugin.h"

#include "pdfpreview.h"

class PDFPreviewPlugin : public DFM_NAMESPACE::DFMFilePreviewPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID DFMFilePreviewFactoryInterface_iid FILE "dde-pdf-preview-plugin.json")

public:
    DFM_NAMESPACE::DFMFilePreview *create(const QString &key) Q_DECL_OVERRIDE;
};

#endif // PDFPREVIEWPLUGIN_H
