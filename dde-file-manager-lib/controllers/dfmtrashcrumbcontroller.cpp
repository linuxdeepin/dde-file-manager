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
#include "dfmtrashcrumbcontroller.h"

#include "dfmcrumbitem.h"

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

QList<CrumbData> DFMTrashCrumbController::seprateUrl(const DUrl &url)
{
    QList<CrumbData> list;

    DAbstractFileInfoPointer info = DFileService::instance()->createFileInfo(nullptr, url);
    DUrlList urlList = info->parentUrlList();
    urlList.insert(0, url);

    DAbstractFileInfoPointer infoPointer;
    // Push urls into crumb list
    DUrlList::const_reverse_iterator iter = urlList.crbegin();
    while (iter != urlList.crend()) {
        const DUrl & oneUrl = *iter;

        QString displayText = oneUrl.fileName();
        // Check for possible display text.
        infoPointer = DFileService::instance()->createFileInfo(nullptr, oneUrl);
        if (infoPointer) {
            displayText = infoPointer->fileDisplayName();
        }
        CrumbData data(oneUrl, displayText);
        list.append(data);

        iter++;
    }

    list[0].setIconFromThemeConfig(QStringLiteral("CrumbIconButton.Trash"));

    return list;
}

DFMCrumbItem *DFMTrashCrumbController::createCrumbItem(const CrumbData &data)
{
    return new DFMCrumbItem(data);
}

QStringList DFMTrashCrumbController::getSuggestList(const QString &text)
{

}

DFM_END_NAMESPACE
