// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfmcrumblistviewmodel.h"
#include "dtkwidget_global.h"

DWIDGET_USE_NAMESPACE

DFM_BEGIN_NAMESPACE

DFMCrumbListviewModel::DFMCrumbListviewModel(QObject *parent)
    :QStandardItemModel (parent)
{

}

DFMCrumbListviewModel::~DFMCrumbListviewModel()
{

}

void DFMCrumbListviewModel::removeAll()
{
    removeRows(0, rowCount());
}

QModelIndex DFMCrumbListviewModel::lastIndex()
{
    return index(rowCount()-1,0);
}

DFM_END_NAMESPACE
