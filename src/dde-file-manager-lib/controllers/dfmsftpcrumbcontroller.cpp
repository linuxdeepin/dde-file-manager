/*
 * Copyright (C) 2020 ~ 2021 Deepin Technology Co., Ltd.
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
#include "dfmsftpcrumbcontroller.h"
#include "dfileservices.h"

#include <QDebug>

DFM_BEGIN_NAMESPACE

DFMSftpCrumbController::DFMSftpCrumbController(QObject *parent)
    : DFMCrumbInterface(parent)
{

}

DFMSftpCrumbController::~DFMSftpCrumbController()
{

}

bool DFMSftpCrumbController::supportedUrl(DUrl url)
{
    return (url.scheme() == SFTP_SCHEME);
}

DFM_END_NAMESPACE
