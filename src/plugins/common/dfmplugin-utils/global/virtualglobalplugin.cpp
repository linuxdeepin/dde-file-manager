// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "virtualglobalplugin.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/file/local/desktopfileinfo.h>
#include <dfm-base/base/standardpaths.h>
#include <dfm-base/mimetype/dmimedatabase.h>
#include <dfm-base/utils/fileutils.h>

using namespace dfmplugin_utils;
DFMBASE_USE_NAMESPACE

static QSharedPointer<dfmbase::FileInfo> transFileInfo(QSharedPointer<dfmbase::FileInfo> fileInfo)
{
    // no translate for gvfs file, invoking suffix/mimeTypeName might cost huge time
    if (fileInfo->urlOf(UrlInfoType::kUrl).path().contains(QRegularExpression(DFMBASE_NAMESPACE::Global::Regex::kGvfsRoot)))
        return fileInfo;

    if (FileUtils::isDesktopFileInfo(fileInfo)) {
        const QUrl &url = fileInfo->urlOf(UrlInfoType::kUrl);
        return FileInfoPointer(new DFMBASE_NAMESPACE::DesktopFileInfo(url, fileInfo));
    }

    return fileInfo;
}

void VirtualGlobalPlugin::initialize()
{
    eventReceiver->initEventConnect();

    DFMBASE_NAMESPACE::InfoFactory::regInfoTransFunc<DFMBASE_NAMESPACE::FileInfo>(DFMBASE_NAMESPACE::Global::Scheme::kFile, transFileInfo);
}

bool VirtualGlobalPlugin::start()
{
    return true;
}
