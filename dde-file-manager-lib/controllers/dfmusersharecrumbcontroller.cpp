/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     Gary Wang <wzc782970009@gmail.com>
 *
 * Maintainer: Gary Wang <wangzichong@deepin.com>
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
