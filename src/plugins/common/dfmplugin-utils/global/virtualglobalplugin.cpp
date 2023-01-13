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
#include "dfm-base/base/standardpaths.h"
#include "dfm-base/mimetype/dmimedatabase.h"

using namespace dfmplugin_utils;
DFMBASE_USE_NAMESPACE

static QSharedPointer<dfmbase::AbstractFileInfo> transFileInfo(QSharedPointer<dfmbase::AbstractFileInfo> fileInfo)
{
    // no translate for gvfs file, invoking suffix/mimeTypeName might cost huge time
    if (fileInfo->urlOf(UrlInfoType::kUrl).path().contains(QRegularExpression(DFMBASE_NAMESPACE::Global::Regex::kGvfsRoot)))
        return fileInfo;

    // At present, there is no dfmio library code. For temporary repair, use the method on v20 to obtain mimeType
    const QString &suffix = fileInfo->nameOf(NameInfoType::kSuffix);
    if (suffix == DFMBASE_NAMESPACE::Global::Scheme::kDesktop
        || fileInfo->urlOf(UrlInfoType::kParentUrl).path() == StandardPaths::location(StandardPaths::StandardLocation::kDesktopPath)) {
        static DMimeDatabase mimedata;
        const QMimeType &mt = mimedata.mimeTypeForFile(fileInfo);
        if (mt.name() == "application/x-desktop"
            && mt.suffixes().contains(DFMBASE_NAMESPACE::Global::Scheme::kDesktop, Qt::CaseInsensitive)) {
            const QUrl &url = fileInfo->urlOf(UrlInfoType::kUrl);
            return DFMLocalFileInfoPointer(new DFMBASE_NAMESPACE::DesktopFileInfo(url));
        }
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
