#include "musicpreviewplugin.h"

DFM_USE_NAMESPACE

DFMFilePreview *MusicPreviewPlugin::create(const QString &key)
{
    Q_UNUSED(key)

    return new MusicPreview();
}
