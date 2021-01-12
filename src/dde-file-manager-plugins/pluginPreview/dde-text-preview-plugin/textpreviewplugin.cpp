#include "textpreviewplugin.h"

DFM_USE_NAMESPACE
DFMFilePreview *TextPreviewPlugin::create(const QString &key)
{
    Q_UNUSED(key)

    return new TextPreview();
}
