// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfmtrashcrumbcontroller.h"
#include "dfileservices.h"
#include "dfileinfo.h"

DFM_BEGIN_NAMESPACE

DFMTrashCrumbController::DFMTrashCrumbController(QObject *parent)
    : DFMCrumbInterface(parent)
{

}

DFMTrashCrumbController::~DFMTrashCrumbController()
{

}

bool DFMTrashCrumbController::supportedUrl(DUrl url)
{
    return (url.scheme() == TRASH_SCHEME);
}

DFM_END_NAMESPACE
