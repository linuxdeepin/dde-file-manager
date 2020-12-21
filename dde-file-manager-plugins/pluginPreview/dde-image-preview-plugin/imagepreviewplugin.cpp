#include "imagepreviewplugin.h"

DFM_USE_NAMESPACE

DFMFilePreview *ImagePreviewPlugin::create(const QString &key)
{
    Q_UNUSED(key)

    return new ImagePreview();
}
