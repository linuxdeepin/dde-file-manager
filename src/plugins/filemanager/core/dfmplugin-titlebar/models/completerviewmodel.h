// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMPLETERVIEWMODEL_H
#define COMPLETERVIEWMODEL_H

#include "dfmplugin_titlebar_global.h"

#include <DStandardItem>

#include <QStandardItemModel>

DWIDGET_USE_NAMESPACE

namespace dfmplugin_titlebar {

class CompleterViewModel : public QStandardItemModel
{
    Q_OBJECT
public:
    explicit CompleterViewModel(QObject *parent = nullptr);
    ~CompleterViewModel();

    void setStringList(const QStringList &list);
    void removeAll();
};
}

#endif   // COMPLETERVIEWMODEL_H
