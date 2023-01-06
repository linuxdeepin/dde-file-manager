// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMCRUMBLISTVIEWDELEGATE_H
#define DFMCRUMBLISTVIEWDELEGATE_H

#include "dfmcrumbinterface.h"
#include "dfmglobal.h"
#include <DStandardItem>
#include <QStandardItemModel>

DFM_BEGIN_NAMESPACE

class DFMCrumbListviewModel : public QStandardItemModel
{
public:
    enum Roles {
        FileUrlRole = Dtk::UserRole + 1
    };
    Q_ENUM(Roles)

    explicit DFMCrumbListviewModel(QObject *parent = nullptr);
    virtual ~DFMCrumbListviewModel();

    void removeAll();
    QModelIndex lastIndex();
};


DFM_END_NAMESPACE

#endif // DFMCRUMBLISTVIEWDELEGATE_H
