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

DPSEARCH_BEGIN_NAMESPACE

class SearchEventReceiver final : public QObject
{
    Q_OBJECT
public:
    static SearchEventReceiver *instance();

public slots:
    void hadleSearch(quint64 winId, const QString &keyword);

private:
    SearchEventReceiver(QObject *parent = nullptr);
};

DPSEARCH_END_NAMESPACE

#endif   // SEARCHEVENTRECEIVER_H
