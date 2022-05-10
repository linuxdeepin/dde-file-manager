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
#ifndef SEARCHSERVICE_H
#define SEARCHSERVICE_H

#include "dfm_filemanager_service_global.h"
#include "search_defines.h"

#include <dfm-framework/service/pluginservicecontext.h>

DSB_FM_BEGIN_NAMESPACE

class SearchServicePrivate;
class SearchService final : public dpf::PluginService, dpf::AutoServiceRegister<SearchService>
{
    Q_OBJECT
    Q_DISABLE_COPY(SearchService)
    friend class dpf::QtClassFactory<dpf::PluginService>;

public:
    static QString name()
    {
        return "org.deepin.service.SearchService";
    }

    static SearchService *service();

    bool search(const QString &taskId, const QUrl &url, const QString &keyword);
    QList<QUrl> matchedResults(const QString &taskId);
    void stop(const QString &taskId);

    bool regCustomSearchInfo(const Search::CustomSearchInfo &info);
    Search::CustomSearchInfo findCustomSearchInfo(const QString &scheme);

signals:
    void matched(const QString &taskId);
    void searchCompleted(const QString &taskId);
    void searchStoped(const QString &taskId);

private:
    explicit SearchService(QObject *parent = nullptr);
    ~SearchService();
    void init();

    QScopedPointer<SearchServicePrivate> d;
};

DSB_FM_END_NAMESPACE

#endif   // SEARCHSERVICE_H
