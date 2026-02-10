// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEPREVIEWFACTORY_H
#define FILEPREVIEWFACTORY_H

#include "dfmplugin_filepreview_global.h"
#include <dfm-base/interfaces/abstractbasepreview.h>

#include <QString>

namespace dfmplugin_filepreview {
class FilePreviewFactory
{
public:
    static QStringList keys();
    static DFMBASE_NAMESPACE::AbstractBasePreview *create(const QString &key);
    static bool isSuitedWithKey(const DFMBASE_NAMESPACE::AbstractBasePreview *view, const QString &key);
    static bool hasPluginForKey(const QString &key);

    static QMap<const DFMBASE_NAMESPACE::AbstractBasePreview *, int> previewToLoaderIndex;
};
}
#endif   // FILEPREVIEWFACTORY_H
