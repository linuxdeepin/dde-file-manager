// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
