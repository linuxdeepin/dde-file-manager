// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfmmasteredmediacrumbcontroller.h"

#include "dfileservices.h"

DFM_BEGIN_NAMESPACE

DFMMasteredMediaCrumbController::DFMMasteredMediaCrumbController(QObject *parent)
    : DFMCrumbInterface(parent)
{

}
DFMMasteredMediaCrumbController::~DFMMasteredMediaCrumbController()
{
}

bool DFMMasteredMediaCrumbController::supportedUrl(DUrl url)
{
    return url.scheme() == BURN_SCHEME;
}

QList<CrumbData> DFMMasteredMediaCrumbController::seprateUrl(const DUrl &url)
{
    QList<CrumbData> ret;
    DUrl cururl(url);
    while (true) {
        DAbstractFileInfoPointer info = DFileService::instance()->createFileInfo(nullptr, cururl);
        if (!info) {
            break;
        }
        QString displayText = info->fileDisplayName();
        ret.push_front(CrumbData(cururl, displayText));
        if (info->parentUrl() == DUrl::fromLocalFile(QDir::homePath())) {
            ret.front().iconName = "media-optical-symbolic";
            break;
        }
        cururl = info->parentUrl();
    }
    return ret;
}

DFM_END_NAMESPACE
