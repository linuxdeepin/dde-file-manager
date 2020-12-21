#ifndef IMAGEPREVIEWPLUGIN_H
#define IMAGEPREVIEWPLUGIN_H


#include "dfmfilepreviewplugin.h"
#include "imagepreview.h"

class ImagePreviewPlugin : public DFM_NAMESPACE::DFMFilePreviewPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID DFMFilePreviewFactoryInterface_iid FILE "dde-image-preview-plugin.json")

public:
    DFM_NAMESPACE::DFMFilePreview *create(const QString &key) Q_DECL_OVERRIDE;
};

#endif // IMAGEPREVIEWPLUGIN_H
