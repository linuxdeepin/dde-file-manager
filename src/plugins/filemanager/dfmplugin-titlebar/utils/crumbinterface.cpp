/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#include "crumbinterface.h"

#include "dfm-base/base/urlroute.h"

DPTITLEBAR_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

CrumbInterface::CrumbInterface(QObject *parent)
    : QObject(parent)
{
}

void CrumbInterface::setKeepAddressBar(bool keep)
{
    keepAddr = keep;
}

void CrumbInterface::processAction(CrumbInterface::ActionType type)
{
    switch (type) {
    case kEscKeyPressed:
    case kClearButtonPressed:
    case kAddressBarLostFocus:
        emit hideAddressBar();
        break;
    case kPauseButtonClicked:
        emit pauseSearch();
        break;
    }
}

void CrumbInterface::crumbUrlChangedBehavior(const QUrl &url)
{
    if (keepAddr) {
        emit keepAddressBar();
    } else {
        emit hideAddrAndUpdateCrumbs(url);
    }
}

bool CrumbInterface::supportedUrl(const QUrl &url)
{
    if (supportedUrlFunc)
        return supportedUrlFunc(url);

    return false;
}

QList<CrumbData> CrumbInterface::seprateUrl(const QUrl &url)
{
    if (seprateUrlFunc)
        return seprateUrlFunc(url);

    // default method
    QList<CrumbData> list;
    QList<QUrl> urls;
    urls.push_back(url);
    UrlRoute::urlParentList(url, &urls);

    for (int count = urls.size() - 1; count >= 0; count--) {
        QUrl curUrl { urls.at(count) };
        QStringList pathList { curUrl.path().split("/") };
        CrumbData data { curUrl, pathList.isEmpty() ? "" : pathList.last() };
        if (UrlRoute::isRootUrl(curUrl))
            data.iconName = UrlRoute::icon(curUrl.scheme()).name();
        list.append(data);
    }

    return list;
}

void CrumbInterface::requestCompletionList(const QUrl &url)
{
}

void CrumbInterface::cancelCompletionListTransmission()
{
}

void CrumbInterface::registewrSupportedUrlCallback(const supportedUrlCallback &func)
{
    supportedUrlFunc = func;
}

void CrumbInterface::registerSeprateUrlCallback(const seprateUrlCallback &func)
{
    seprateUrlFunc = func;
}
