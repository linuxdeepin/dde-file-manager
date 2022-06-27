/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
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
#ifndef SEARCHEVENTRECEIVER_H
#define SEARCHEVENTRECEIVER_H

#include "dfmplugin_search_global.h"

#include <QObject>

#define SearchEventReceiverIns DPSEARCH_NAMESPACE::SearchEventReceiver::instance()

namespace dfmplugin_search {

class SearchEventReceiver final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(SearchEventReceiver)
public:
    static SearchEventReceiver *instance();

public slots:
    void handleSearch(quint64 winId, const QString &keyword);
    void handleStopSearch(quint64 winId);
    void handleShowAdvanceSearchBar(quint64 winId, bool visible);
    void handleUrlChanged(quint64 winId, const QUrl &u);

private:
    explicit SearchEventReceiver(QObject *parent = nullptr);
};

}

#endif   // SEARCHEVENTRECEIVER_H
