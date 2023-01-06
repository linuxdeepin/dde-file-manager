// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfmusersharecrumbcontroller.h"
#include "controllers/pathmanager.h"
#include "singleton.h"

DFM_BEGIN_NAMESPACE

DFMUserShareCrumbController::DFMUserShareCrumbController(QObject *parent)
    : DFMCrumbInterface(parent)
{

}

DFMUserShareCrumbController::~DFMUserShareCrumbController()
{

}

bool DFMUserShareCrumbController::supportedUrl(DUrl url)
{
    return (url.scheme() == USERSHARE_SCHEME);
}

QList<CrumbData> DFMUserShareCrumbController::seprateUrl(const DUrl &url)
{
    Q_UNUSED(url);
    return { CrumbData(DUrl(USERSHARE_ROOT),
                       Singleton<PathManager>::instance()->getSystemPathDisplayName("UserShare"),
                       Singleton<PathManager>::instance()->getSystemPathIconName("UserShare")) };
}

DFM_END_NAMESPACE
