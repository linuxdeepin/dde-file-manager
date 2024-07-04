// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "crumbmodel.h"
#include "dtkwidget_global.h"

using namespace dfmplugin_titlebar;
DWIDGET_USE_NAMESPACE

CrumbModel::CrumbModel(QObject *parent)
    : QStandardItemModel(parent)
{
}

CrumbModel::~CrumbModel()
{
}

void CrumbModel::removeAll()
{
    removeRows(0, rowCount());
}

QModelIndex CrumbModel::lastIndex()
{
    return index(rowCount() - 1, 0);
}
