// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CUSTOMMODE_P_H
#define CUSTOMMODE_P_H

#include "mode/custommode.h"
#include "customdatahandler.h"
#include "collection/collectionholder.h"
#include "mode/selectionsynchelper.h"

#include <QTimer>

namespace ddplugin_organizer {

class CustomModePrivate
{
public:
    explicit CustomModePrivate(CustomMode *qq);
    ~CustomModePrivate();
    CustomDataHandler *dataHandler = nullptr;
    QHash<QString, CollectionHolderPointer> holders;
    QTimer dataSyncTimer;
    ItemSelectionModel *selectionModel = nullptr;
    SelectionSyncHelper *selectionHelper = nullptr;
private:
    CustomMode *q;
};

}

#endif // CUSTOMMODE_P_H
