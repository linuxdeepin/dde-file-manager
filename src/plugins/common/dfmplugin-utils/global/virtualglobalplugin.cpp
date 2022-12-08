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
#include "virtualglobalplugin.h"

#include "dfm-base/base/schemefactory.h"
#include "dfm-base/dfm_global_defines.h"
#include "dfm-base/file/local/desktopfileinfo.h"

using namespace dfmplugin_utils;

static QSharedPointer<dfmbase::AbstractFileInfo> transFileInfo(QSharedPointer<dfmbase::AbstractFileInfo> fileInfo)
{
    // no translate for gvfs file, invoking suffix/mimeTypeName might cost huge time
    if (fileInfo->urlInfo(UrlInfo::kUrl).path().contains(QRegularExpression(DFMBASE_NAMESPACE::Global::Regex::kGvfsRoot)))
        return fileInfo;

    const QString &suffix = fileInfo->nameInfo(NameInfo::kSuffix);
    const QString &mimeTypeName = fileInfo->nameInfo(NameInfo::kMimeTypeName);
    if (suffix == DFMBASE_NAMESPACE::Global::Scheme::kDesktop && mimeTypeName == "application/x-desktop") {
        const QUrl &url = fileInfo->urlInfo(UrlInfo::kUrl);
        return DFMLocalFileInfoPointer(new DFMBASE_NAMESPACE::DesktopFileInfo(url));
    }
    return fileInfo;
}

void VirtualGlobalPlugin::initialize()
{
    eventReceiver->initEventConnect();

    DFMBASE_NAMESPACE::InfoFactory::regInfoTransFunc<DFMBASE_NAMESPACE::AbstractFileInfo>(DFMBASE_NAMESPACE::Global::Scheme::kFile, transFileInfo);
}

bool VirtualGlobalPlugin::start()
{
    return true;
}
