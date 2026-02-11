// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
    void handleAddressInputStr(quint64 windId, QString *str);
    void handleFileAdd(const QUrl &url);
    void handleFileDelete(const QUrl &url);
    void handleFileRename(const QUrl &oldUrl, const QUrl &newUrl);

private:
    explicit SearchEventReceiver(QObject *parent = nullptr);
};

}

#endif   // SEARCHEVENTRECEIVER_H
