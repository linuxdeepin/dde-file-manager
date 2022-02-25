#ifndef FILEPREVIEWPLUGIN_H
#define FILEPREVIEWPLUGIN_H

#include "dfm_base_global.h"

#include <QObject>

DFMBASE_BEGIN_NAMESPACE
#define FilePreviewFactoryInterface_iid "com.deepin.filemanager.FilePreviewFactoryInterface_iid"

class AbstractBasePreview;
class AbstractFilePreviewPlugin : public QObject
{
    Q_OBJECT
public:
    explicit AbstractFilePreviewPlugin(QObject *parent = nullptr);

    virtual AbstractBasePreview *create(const QString &key) = 0;
};
DFMBASE_END_NAMESPACE
#endif   // FILEPREVIEWPLUGIN_H
