#ifndef FILEVIEWITEM_P_H
#define FILEVIEWITEM_P_H

#include "dfm-base/widgets/dfmfileview/fileviewitem.h"

class FileViewItem;
class FileViewItemPrivate
{
    Q_DECLARE_PUBLIC(FileViewItem)
    FileViewItem * const q_ptr;

    AbstractFileInfoPointer fileinfo;
    QMimeType mimeType;

public:
    explicit FileViewItemPrivate(FileViewItem* qq)
        : q_ptr(qq)
    {

    }
};

#endif // FILEVIEWITEM_P_H
