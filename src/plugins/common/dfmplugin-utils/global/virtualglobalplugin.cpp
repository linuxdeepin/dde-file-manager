// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "virtualglobalplugin.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/file/local/desktopfileinfo.h>
#include <dfm-base/base/standardpaths.h>
#include <dfm-base/mimetype/dmimedatabase.h>

using namespace dfmplugin_utils;
DFMBASE_USE_NAMESPACE

static QSharedPointer<dfmbase::FileInfo> transFileInfo(QSharedPointer<dfmbase::FileInfo> fileInfo)
{
    // no translate for gvfs file, invoking suffix/mimeTypeName might cost huge time
    if (fileInfo->urlOf(UrlInfoType::kUrl).path().contains(QRegularExpression(DFMBASE_NAMESPACE::Global::Regex::kGvfsRoot)))
        return fileInfo;

    // At present, there is no dfmio library code. For temporary repair, use the method on v20 to obtain mimeType
    const QString &suffix = fileInfo->nameOf(NameInfoType::kSuffix);
    if (suffix == DFMBASE_NAMESPACE::Global::Scheme::kDesktop
        || fileInfo->urlOf(UrlInfoType::kParentUrl).path() == StandardPaths::location(StandardPaths::StandardLocation::kDesktopPath)) {
        const QUrl &url = fileInfo->urlOf(UrlInfoType::kUrl);
        QMimeType mt = fileInfo->fileMimeType();
        if (!mt.isValid())
            mt = DMimeDatabase().mimeTypeForFile(url.path(),QMimeDatabase::MatchDefault, QString());

        if (mt.name() == "application/x-desktop"
            && mt.suffixes().contains(DFMBASE_NAMESPACE::Global::Scheme::kDesktop, Qt::CaseInsensitive)) {
            return FileInfoPointer(new DFMBASE_NAMESPACE::DesktopFileInfo(url, fileInfo));
        }
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
