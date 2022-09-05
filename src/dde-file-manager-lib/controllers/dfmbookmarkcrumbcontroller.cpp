// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfmbookmarkcrumbcontroller.h"

DFM_BEGIN_NAMESPACE

DFMBookmarkCrumbController::DFMBookmarkCrumbController(QObject *parent)
    : DFMCrumbInterface(parent)
{

}

DFMBookmarkCrumbController::~DFMBookmarkCrumbController()
{

}

bool DFMBookmarkCrumbController::supportedUrl(DUrl url)
{
    return (url.scheme() == BOOKMARK_SCHEME);
}

QList<CrumbData> DFMBookmarkCrumbController::seprateUrl(const DUrl &url)
{
    Q_UNUSED(url);
    return { CrumbData(DUrl(BOOKMARK_ROOT), QObject::tr("Bookmarks"), "CrumbIconButton.Bookmarks") };
}

DFM_END_NAMESPACE
