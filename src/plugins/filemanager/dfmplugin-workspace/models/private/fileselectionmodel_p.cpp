// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fileselectionmodel_p.h"

#include <QItemSelectionModel>

using namespace dfmplugin_workspace;

FileSelectionModelPrivate::FileSelectionModelPrivate(FileSelectionModel *qq)
    : QObject(qq),
      q(qq)
{
    timer.setSingleShot(true);
    QObject::connect(&timer, &QTimer::timeout, q, &FileSelectionModel::updateSelecteds);
}
