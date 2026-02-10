// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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

OrganizerBroker::OrganizerBroker(QObject *parent)
    : QObject(parent)
{
}

OrganizerBroker::~OrganizerBroker()
{
    OrganizerBrokerDisconnect(slot_CollectionView_GridPoint);
    OrganizerBrokerDisconnect(slot_CollectionView_VisualRect);
    OrganizerBrokerDisconnect(slot_CollectionView_View);
    OrganizerBrokerDisconnect(slot_CollectionItemDelegate_IconRect);
    OrganizerBrokerDisconnect(slot_CollectionModel_Refresh);
    OrganizerBrokerDisconnect(slot_CollectionModel_SelectAll);
}

bool OrganizerBroker::init()
{
    OrganizerBrokerSlot(slot_CollectionView_GridPoint, &OrganizerBroker::gridPoint);
    OrganizerBrokerSlot(slot_CollectionView_VisualRect, &OrganizerBroker::visualRect);
    OrganizerBrokerSlot(slot_CollectionView_View, &OrganizerBroker::view);
    OrganizerBrokerSlot(slot_CollectionItemDelegate_IconRect, &OrganizerBroker::iconRect);
    OrganizerBrokerSlot(slot_CollectionModel_Refresh, &OrganizerBroker::refreshModel);
    OrganizerBrokerSlot(slot_CollectionModel_SelectAll, &OrganizerBroker::selectAllItems);

    return true;
}
