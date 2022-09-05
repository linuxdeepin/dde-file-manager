// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfmsmbcrumbcontroller.h"
#include "dfileservices.h"

#include <QDebug>

DFM_BEGIN_NAMESPACE

DFMSmbCrumbController::DFMSmbCrumbController(QObject *parent)
    : DFMCrumbInterface(parent)
{

}

DFMSmbCrumbController::~DFMSmbCrumbController()
{

}

bool DFMSmbCrumbController::supportedUrl(DUrl url)
{
    return (url.scheme() == SMB_SCHEME);
}

DFM_END_NAMESPACE
