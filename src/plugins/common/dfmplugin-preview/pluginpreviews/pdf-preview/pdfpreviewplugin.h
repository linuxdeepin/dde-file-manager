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

#ifndef PDFPREVIEWPLUGIN_H
#define PDFPREVIEWPLUGIN_H
#include "preview_plugin_global.h"
#include "dfm-base/interfaces/abstractfilepreviewplugin.h"

#include "pdfpreview.h"

PREVIEW_BEGIN_NAMESPACE
class PDFPreviewPlugin : public DFMBASE_NAMESPACE::AbstractFilePreviewPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID FilePreviewFactoryInterface_iid FILE "dde-pdf-preview-plugin.json")

public:
    DFMBASE_NAMESPACE::AbstractBasePreview *create(const QString &key) Q_DECL_OVERRIDE;
};
PREVIEW_END_NAMESPACE
#endif   // PDFPREVIEWPLUGIN_H
