// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfmcomputercrumbcontroller.h"
#include "controllers/pathmanager.h"
#include "singleton.h"

DFM_BEGIN_NAMESPACE

DFMComputerCrumbController::DFMComputerCrumbController(QObject *parent)
    : DFMCrumbInterface(parent)
{

}

DFMComputerCrumbController::~DFMComputerCrumbController()
{

}

bool DFMComputerCrumbController::supportedUrl(DUrl url)
{
    return (url.scheme() == COMPUTER_SCHEME);
}

QList<CrumbData> DFMComputerCrumbController::seprateUrl(const DUrl &url)
{
    Q_UNUSED(url);
    QString text = Singleton<PathManager>::instance()->getSystemPathDisplayName("Computer");
    QString iconName = Singleton<PathManager>::instance()->getSystemPathIconName("Computer");
    return { CrumbData(DUrl::fromComputerFile("/"), text, iconName) };
}

DFM_END_NAMESPACE
