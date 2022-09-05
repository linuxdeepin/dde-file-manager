// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
