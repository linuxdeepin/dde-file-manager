/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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
#include "organizerbroker.h"
#include "ddplugin_organizer_global.h"

#include <dfm-framework/dpf.h>

#include <QAbstractItemView>

Q_DECLARE_METATYPE(QPoint *)

using namespace ddplugin_organizer;

#define OrganizerBrokerSlot(topic, args...) \
            dpfSlotChannel->connect(QT_STRINGIFY(DDP_ORGANIZER_NAMESPACE), QT_STRINGIFY2(topic), this, ##args)

#define OrganizerBrokerDisconnect(topic) \
            dpfSlotChannel->disconnect(QT_STRINGIFY(DDP_ORGANIZER_NAMESPACE), QT_STRINGIFY2(topic))

OrganizerBroker::OrganizerBroker(QObject *parent) : QObject(parent)
{

}

OrganizerBroker::~OrganizerBroker()
{
    OrganizerBrokerDisconnect(slot_CollectionView_GridPoint);
    OrganizerBrokerDisconnect(slot_CollectionView_VisualRect);
    OrganizerBrokerDisconnect(slot_CollectionView_View);
    OrganizerBrokerDisconnect(slot_CollectionItemDelegate_IconRect);
    OrganizerBrokerDisconnect(slot_CollectionModel_Refresh);
}

bool OrganizerBroker::init()
{
    OrganizerBrokerSlot(slot_CollectionView_GridPoint, &OrganizerBroker::gridPoint);
    OrganizerBrokerSlot(slot_CollectionView_VisualRect, &OrganizerBroker::visualRect);
    OrganizerBrokerSlot(slot_CollectionView_View, &OrganizerBroker::view);
    OrganizerBrokerSlot(slot_CollectionItemDelegate_IconRect, &OrganizerBroker::iconRect);
    OrganizerBrokerSlot(slot_CollectionModel_Refresh, &OrganizerBroker::refreshModel);
    return true;
}
