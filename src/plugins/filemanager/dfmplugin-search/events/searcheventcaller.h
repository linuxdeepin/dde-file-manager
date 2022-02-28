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
#ifndef SEARCHEVENTCALLER_H
#define SEARCHEVENTCALLER_H

#include "dfmplugin_search_global.h"

#include <QObject>

DPSEARCH_BEGIN_NAMESPACE

class SearchEventCaller
{
public:
    static void sendDoSearch(quint64 winId, const QUrl &url);
    static void sendShowAdvanceSearchBar(quint64 winId, bool visible);
    static void sendStartSpinner(quint64 winId);
    static void sendStopSpinner(quint64 winId);

private:
    SearchEventCaller() = delete;
};

DPSEARCH_END_NAMESPACE

#endif   // SEARCHEVENTCALLER_H
