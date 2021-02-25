/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     Li yigang <liyigang@uniontech.com>
 *
 * Maintainer: Li yigang <liyigang@uniontech.com>
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
#include "dfmftpcrumbcontroller.h"
#include "dfileservices.h"

DFM_BEGIN_NAMESPACE

DFMFtpCrumbController::DFMFtpCrumbController(QObject *parent)
    : DFMCrumbInterface(parent)
{

}

DFMFtpCrumbController::~DFMFtpCrumbController()
{

}

bool DFMFtpCrumbController::supportedUrl(DUrl url)
{
    return (url.scheme() == FTP_SCHEME);
}

DFM_END_NAMESPACE
