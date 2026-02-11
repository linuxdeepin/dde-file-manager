// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
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

FileSelectionModelPrivate::~FileSelectionModelPrivate()
{
    // 停止定时器，防止在析构过程中定时器触发
    if (timer.isActive()) {
        timer.stop();
    }

    // 断开定时器的所有信号连接
    timer.disconnect();

    // 清理其他资源
    selection.clear();
    firstSelectedIndex = QPersistentModelIndex();
    lastSelectedIndex = QPersistentModelIndex();
    selectedList.clear();
}
