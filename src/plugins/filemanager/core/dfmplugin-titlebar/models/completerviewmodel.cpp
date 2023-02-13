// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "completerviewmodel.h"

#include <QAbstractItemView>

using namespace dfmplugin_titlebar;

CompleterViewModel::CompleterViewModel(QObject *parent)
    : QStandardItemModel(parent)
{
}

CompleterViewModel::~CompleterViewModel()
{
}

void CompleterViewModel::setStringList(const QStringList &list)
{
    removeAll();
    for (const auto &str : list) {
        if (str.isEmpty())
            continue;

        QStandardItem *item = new QStandardItem(str);
        appendRow(item);
    }
}

void CompleterViewModel::removeAll()
{
    removeRows(0, rowCount());
}
