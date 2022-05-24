/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#include "utils.h"
#include "events/globaleventreceiver.h"

#include "dfm-base/base/schemefactory.h"
#include "dfm-base/dfm_global_defines.h"
#include "dfm-base/file/local/desktopfileinfo.h"

static QSharedPointer<dfmbase::AbstractFileInfo> transFileInfo(QSharedPointer<dfmbase::AbstractFileInfo> fileInfo)
{
    const QString &suffix = fileInfo->suffix();
    const QString &mimeTypeName = fileInfo->mimeTypeName();
    if (suffix == DFMBASE_NAMESPACE::Global::kDesktop && mimeTypeName == "application/x-desktop") {
        const QUrl &url = fileInfo->url();
        return DFMLocalFileInfoPointer(new DFMBASE_NAMESPACE::DesktopFileInfo(url));
    }
    return fileInfo;
}

bool dfmplugin_utils::Utils::start()
{
    GlobalEventReceiver::instance()->initEventConnect();

    DFMBASE_NAMESPACE::InfoFactory::regInfoTransFunc<DFMBASE_NAMESPACE::AbstractFileInfo>(DFMBASE_NAMESPACE::Global::kFile, transFileInfo);
    return true;
}
