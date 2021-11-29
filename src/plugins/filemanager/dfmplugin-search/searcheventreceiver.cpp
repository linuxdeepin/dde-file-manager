/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuzhangjian<liuzhangjian@uniontech.com>
 *
 * Maintainer: liuzhangjian<liuzhangjian@uniontech.com>
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
#include "searcheventreceiver.h"
#include "searchutils.h"
#include "searcheventcaller.h"

#include "window/windowservice.h"

void SearchEventReceiver::eventProcess(const dpf::Event &event)
{
    QString eventTopic = event.topic();
    if (eventTopic == EventTypes::kTopicSearchEvent)
        searchEvent(event);
}

void SearchEventReceiver::searchEvent(const dpf::Event &event)
{
    if (event.data() == EventTypes::kDataSearch) {
        auto &ctx = dpfInstance.serviceContext();
        WindowService *windowService = ctx.service<WindowService>(WindowService::name());
        dpfDebug() << Q_FUNC_INFO << windowService;

        if (windowService) {
            const auto targetUrl = event.property(EventTypes::kPropertyTargetUrl).toUrl();
            const auto keyword = event.property(EventTypes::kPropertyKeyword).toString();
            const auto winIdx = event.property(EventTypes::kPropertyKeyWindowIndex).toULongLong();

            // 构建搜索url
            QUrl searchUrl;
            if (UrlRoute::isVirtual(targetUrl)) {
                if (SearchUtils::isSearchFile(targetUrl)) {
                    searchUrl = SearchUtils::setSearchKeyword(targetUrl, keyword);
                } else {
                    searchUrl = SearchUtils::fromSearchFile(targetUrl, keyword, QUrl());
                }
            } else {
                QString targetPath = UrlRoute::urlToPath(targetUrl);
                searchUrl = SearchUtils::fromSearchFile(QUrl(targetPath), keyword, QUrl());
            }

            // 发送setRootUrl事件，core响应
            SearchEventCaller::sendSetRootUrlEvent(searchUrl, winIdx);
        }
    }
}
