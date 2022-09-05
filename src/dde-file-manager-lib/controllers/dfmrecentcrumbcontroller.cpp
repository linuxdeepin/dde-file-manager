// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfmrecentcrumbcontroller.h"
#include "dfileservices.h"
#include "pathmanager.h"
#include "singleton.h"

#include <QDebug>

DFM_BEGIN_NAMESPACE

DFMRecentCrumbController::DFMRecentCrumbController(QObject *parent)
    : DFMCrumbInterface(parent)
{

}

bool DFMRecentCrumbController::supportedUrl(DUrl url)
{
    return (url.scheme() == RECENT_SCHEME);
}

QList<CrumbData> DFMRecentCrumbController::seprateUrl(const DUrl &url)
{
    Q_UNUSED(url);
    QString displayText = Singleton<PathManager>::instance()->getSystemPathDisplayName("Recent");
    QString iconName = Singleton<PathManager>::instance()->getSystemPathIconName("Recent");
    return { CrumbData(DUrl(RECENT_ROOT), displayText, iconName) };
}

DFM_END_NAMESPACE
