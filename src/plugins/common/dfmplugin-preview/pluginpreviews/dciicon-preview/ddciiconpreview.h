/*
 * Copyright (C) 2022 UnionTech Technology Co., Ltd.
 *
 * Author:     Chen Bin <chenbin@uniontech.com>
 *
 * Maintainer: Chen Bin <chenbin@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DDCIICONPREVIEW_H
#define DDCIICONPREVIEW_H

#include "preview_plugin_global.h"
#include "dfm-base/interfaces/abstractfilepreviewplugin.h"

DFMBASE_BEGIN_NAMESPACE
class AbstractBasePreview;
DFMBASE_END_NAMESPACE

namespace plugin_filepreview {
class DDciIconPreviewPlugin : public DFMBASE_NAMESPACE::AbstractFilePreviewPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID FilePreviewFactoryInterface_iid FILE "dtk-dciicon-preview-plugin.json")

public:
    DFMBASE_NAMESPACE::AbstractBasePreview *create(const QString &) override;
};
}   // namespace plugin_filepreview

#endif   // DDCIICONPREVIEW_H
