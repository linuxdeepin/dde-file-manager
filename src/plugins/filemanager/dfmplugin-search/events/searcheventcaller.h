// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SEARCHEVENTCALLER_H
#define SEARCHEVENTCALLER_H

#include "dfmplugin_search_global.h"

#include <QObject>

namespace dfmplugin_search {

class SearchEventCaller
{
public:
    static void sendChangeCurrentUrl(quint64 winId, const QUrl &url);
    static void sendShowAdvanceSearchBar(quint64 winId, bool visible);
    static void sendShowAdvanceSearchButton(quint64 winId, bool visible);
    static void sendStartSpinner(quint64 winId);
    static void sendStopSpinner(quint64 winId);

private:
    SearchEventCaller() = delete;
};

}

#endif   // SEARCHEVENTCALLER_H
