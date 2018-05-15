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
#include "dfmcrumbinterface.h"

#include "dfmcrumbitem.h"

#include "dfileservices.h"
#include "dfileinfo.h"

DFM_BEGIN_NAMESPACE

CrumbData::CrumbData(DUrl url, QString displayText, QString iconName, QString iconKey)
{
    this->url = url;
    this->displayText = displayText;
    this->iconName = iconName;
    this->iconKey = iconKey;
}

void CrumbData::setIconFromThemeConfig(QString iconName, QString iconKey)
{
    this->iconName = iconName;
    this->iconKey = iconKey;
}

DFMCrumbInterface::DFMCrumbInterface(QObject *parent)
    : QObject(parent)
{

}

DFMCrumbInterface::~DFMCrumbInterface()
{

}

QList<CrumbData> DFMCrumbInterface::seprateUrl(const DUrl &url)
{
    QList<CrumbData> list;

    DAbstractFileInfoPointer info = DFileService::instance()->createFileInfo(nullptr, url);
    DUrlList urlList;
    urlList.append(url);
    urlList.append(info->parentUrlList());

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

    return list;
}

DFMCrumbItem *DFMCrumbInterface::createCrumbItem(const CrumbData &data)
{
    return new DFMCrumbItem(data);
}

DFM_END_NAMESPACE
