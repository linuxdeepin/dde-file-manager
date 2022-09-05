// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfmnetworkcrumbcontroller.h"
#include "pathmanager.h"

#include "singleton.h"

DFM_BEGIN_NAMESPACE

DFMNetworkCrumbController::DFMNetworkCrumbController(QObject *parent)
    : DFMCrumbInterface(parent)
{

}

DFMNetworkCrumbController::~DFMNetworkCrumbController()
{

}

bool DFMNetworkCrumbController::supportedUrl(DUrl url)
{
    return (url.scheme() == NETWORK_SCHEME);
}

QList<CrumbData> DFMNetworkCrumbController::seprateUrl(const DUrl &url)
{
    Q_UNUSED(url);
    QString displayText = Singleton<PathManager>::instance()->getSystemPathDisplayName("Network");
    QString iconName = Singleton<PathManager>::instance()->getSystemPathIconName("Network");
    return { CrumbData(DUrl(NETWORK_ROOT), displayText, iconName) };
}

DFM_END_NAMESPACE
