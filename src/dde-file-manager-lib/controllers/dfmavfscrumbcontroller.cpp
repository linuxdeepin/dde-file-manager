// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfmavfscrumbcontroller.h"

#include "dfileservices.h"
#include "dfileinfo.h"
DFM_BEGIN_NAMESPACE

DFMAvfsCrumbController::DFMAvfsCrumbController(QObject *parent)
    : DFMFileCrumbController(parent)
{

}

DFMAvfsCrumbController::~DFMAvfsCrumbController()
{

}

bool DFMAvfsCrumbController::supportedUrl(DUrl url)
{
    return (url.scheme() == AVFS_SCHEME);
}

DFM_END_NAMESPACE
