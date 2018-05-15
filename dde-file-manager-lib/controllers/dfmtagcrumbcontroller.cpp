/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     Gary Wang <wzc782970009@gmail.com>
 *
 * Maintainer: Gary Wang <wangzichong@deepin.com>
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
#include "dfmtagcrumbcontroller.h"

#include "dfmcrumbitem.h"

#include "tag/tagmanager.h"

DFM_BEGIN_NAMESPACE

DFMTagCrumbController::DFMTagCrumbController(QObject *parent)
    : DFMCrumbInterface(parent)
{

}

DFMTagCrumbController::~DFMTagCrumbController()
{

}

bool DFMTagCrumbController::supportedUrl(DUrl url)
{
    return (url.scheme() == TAG_SCHEME);
}

QList<CrumbData> DFMTagCrumbController::seprateUrl(const DUrl &url)
{
    QString colorName = TagManager::instance()->getTagColorName(url.fileName());
    colorName = ("BookmarkItem." + colorName);
    return { CrumbData(url, url.fileName(), colorName) };
}

DFMCrumbItem *DFMTagCrumbController::createCrumbItem(const CrumbData &data)
{
    return new DFMCrumbItem(data);
}

DFMCrumbItem *DFMTagCrumbController::createCrumbItem(const DUrl &url)
{

}

QStringList DFMTagCrumbController::getSuggestList(const QString &text)
{

}

DFM_END_NAMESPACE
