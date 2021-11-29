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
#ifndef SEARCHEVENTRECEIVER_H
#define SEARCHEVENTRECEIVER_H

#include "window/contexts.h" // TODO(zhangs): hide

#include <dfm-framework/framework.h>

DSB_FM_USE_NAMESPACE

class SearchEventReceiver : public dpf::EventHandler, dpf::AutoEventHandlerRegister<SearchEventReceiver>
{
    Q_OBJECT
public:
    static inline EventHandler::Type type()
    {
        return EventHandler::Type::Sync;
    }

    static inline QStringList topics()
    {
        return QStringList() << EventTypes::kTopicSearchEvent;
    }

    SearchEventReceiver() : AutoEventHandlerRegister<SearchEventReceiver>() {}
    void eventProcess(const dpf::Event &event) override;
    void searchEvent(const dpf::Event &event);
};

#endif // SEARCHEVENTRECEIVER_H
