// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CrumbModel_H
#define CrumbModel_H

#include "dfmplugin_titlebar_global.h"

#include <DStandardItem>
#include <QStandardItemModel>

DWIDGET_USE_NAMESPACE

namespace dfmplugin_titlebar {

class CrumbModel : public QStandardItemModel
{
    Q_OBJECT
public:
    enum Roles {
        FileUrlRole = Dtk::UserRole + 1,
        FullUrlRole
    };
    Q_ENUM(Roles)

    explicit CrumbModel(QObject *parent = nullptr);
    virtual ~CrumbModel();

    void removeAll();
    QModelIndex lastIndex();
};

}

#endif   // CrumbModel_H
