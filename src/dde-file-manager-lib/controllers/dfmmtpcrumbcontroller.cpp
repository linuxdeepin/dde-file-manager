// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfmmtpcrumbcontroller.h"

DFM_BEGIN_NAMESPACE

DFMMtpCrumbController::DFMMtpCrumbController(QObject *parent)
    : DFMCrumbInterface(parent)
{

}

DFMMtpCrumbController::~DFMMtpCrumbController()
{

}

bool DFMMtpCrumbController::supportedUrl(DUrl url)
{
    return (url.scheme() == MTP_SCHEME);
}

DFM_END_NAMESPACE
