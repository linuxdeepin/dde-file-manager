#include "videopreviewplugin.h"

DFM_USE_NAMESPACE

DFMFilePreview *VideoPreviewPlugin::create(const QString &key)
{
    Q_UNUSED(key)

    return new VideoPreview();
}
