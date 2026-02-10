// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "itemselectionmodel.h"
#include "models/collectionmodel.h"

using namespace ddplugin_organizer;

void ItemSelectionModel::selectAll()
{
    auto m = dynamic_cast<CollectionModel *>(model());
    if (!m)
        return;

    const int row = m->rowCount(m->rootIndex());
    if (row < 1)
        return;

    QItemSelection allIndex(m->index(0, 0), m->index(row - 1, 0));
    select(allIndex, QItemSelectionModel::ClearAndSelect);
}
