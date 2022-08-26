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
#include "filepreview.h"
#include "events/fileprevieweventreceiver.h"

#include "dfm-base/base/configs/dconfig/dconfigmanager.h"

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_filepreview;

void FilePreview::initialize()
{
    FilePreviewEventReceiver::instance()->connectService();
}

bool FilePreview::start()
{
    QString err;
    auto ret = DConfigManager::instance()->addConfig("org.deepin.dde.file-manager.preview", &err);
    if (!ret)
        qWarning() << "create dconfig failed: " << err;
    return true;
}
