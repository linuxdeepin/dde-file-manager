// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef ITEMSELECTIONMODEL_H
#define ITEMSELECTIONMODEL_H

#include <QItemSelectionModel>

namespace ddplugin_organizer {

class ItemSelectionModel : public QItemSelectionModel
{
    Q_OBJECT
public:
    using QItemSelectionModel::QItemSelectionModel;
    void selectAll();
};

}

#endif // ITEMSELECTIONMODEL_H
