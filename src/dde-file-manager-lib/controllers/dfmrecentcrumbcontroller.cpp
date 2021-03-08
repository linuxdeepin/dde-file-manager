/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
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
