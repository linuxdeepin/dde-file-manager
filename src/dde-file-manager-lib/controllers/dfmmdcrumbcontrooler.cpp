// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfmmdcrumbcontrooler.h"

DFM_BEGIN_NAMESPACE

DFMMDCrumbControoler::DFMMDCrumbControoler(QObject *parent)
    : DFMCrumbInterface(parent)
{

}

DFMMDCrumbControoler::~DFMMDCrumbControoler()
{

}

bool DFMMDCrumbControoler::supportedUrl(DUrl url)
{
    return (url.scheme() == DFMMD_SCHEME);
}

DFM_END_NAMESPACE
