/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: lixiang<lixianga@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef FILEPREVIEWFACTORY_H
#define FILEPREVIEWFACTORY_H

#include "dfmplugin_filepreview_global.h"
#include "dfm-base/interfaces/abstractbasepreview.h"

#include <QString>

namespace dfmplugin_filepreview {
class FilePreviewFactory
{
public:
    static QStringList keys();
    static DFMBASE_NAMESPACE::AbstractBasePreview *create(const QString &key);
    static bool isSuitedWithKey(const DFMBASE_NAMESPACE::AbstractBasePreview *view, const QString &key);

    static QMap<const DFMBASE_NAMESPACE::AbstractBasePreview *, int> previewToLoaderIndex;
};
}
#endif   // FILEPREVIEWFACTORY_H
