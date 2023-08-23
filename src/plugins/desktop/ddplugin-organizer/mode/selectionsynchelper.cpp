// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "selectionsynchelper.h"

#include <QDebug>

using namespace ddplugin_organizer;

SelectionSyncHelper::SelectionSyncHelper(QObject *parent) : QObject(parent)
{

}

void SelectionSyncHelper::setInnerModel(ItemSelectionModel *sel)
{
    if (inner)
        disconnect(inner, nullptr, this, nullptr);

    if (sel) {
        qDebug() << "set inner selection model." << sel;
        inner = sel;
        connect(inner, &ItemSelectionModel::destroyed, this, &SelectionSyncHelper::innerModelDestroyed);
        connect(inner, &ItemSelectionModel::selectionChanged, this, &SelectionSyncHelper::clearExteralSelection);
    } else {
        qWarning() << "set inner selection model to null";
        inner = nullptr;
    }
}

void SelectionSyncHelper::setExternalModel(QItemSelectionModel *sel)
{
    if (external)
        disconnect(external, nullptr, this, nullptr);

    if (sel) {
        qDebug() << "set external selection model." << external;
        external = sel;
        connect(external, &QItemSelectionModel::destroyed, this, &SelectionSyncHelper::externalModelDestroyed);
        connect(external, &QItemSelectionModel::selectionChanged, this, &SelectionSyncHelper::clearInnerSelection);
    } else {
        qWarning() << "set external selection model to null";
        external = nullptr;
    }
}

void SelectionSyncHelper::setShell(CanvasSelectionShell *sh)
{
    if (shell)
        disconnect(shell, nullptr, this, nullptr);

    shell = sh;
    if (!sh)
        return;

    // clear selection
    connect(sh, &CanvasSelectionShell::requestClear, this, &SelectionSyncHelper::clearInnerSelection);
}

void SelectionSyncHelper::clearExteralSelection()
{
    qDebug() << "innerSelectionChanged";
    if (!enabled || !external || !external->hasSelection())
        return;

    // disconnect to prevent signal recursion
    disconnect(external, &QItemSelectionModel::selectionChanged, this, &SelectionSyncHelper::clearInnerSelection);

    // clear canvas selection if any selection was operated in collection.
    external->clear();

    connect(external, &QItemSelectionModel::selectionChanged, this, &SelectionSyncHelper::clearInnerSelection);
}

void SelectionSyncHelper::clearInnerSelection()
{
    qDebug() << "externalSelectionChanged";
    if (!enabled || !inner || !inner->hasSelection())
        return;

    // disconnect to prevent signal recursion
    disconnect(inner, &QItemSelectionModel::selectionChanged, this, &SelectionSyncHelper::clearExteralSelection);

    // clear collection selection if any selection was operated in canvas.
    inner->clear();

    connect(inner, &QItemSelectionModel::selectionChanged, this, &SelectionSyncHelper::clearExteralSelection);
}

void SelectionSyncHelper::innerModelDestroyed()
{
    qInfo() << "inner selection is destroyed.";
    inner = nullptr;
}

void SelectionSyncHelper::externalModelDestroyed()
{
    qInfo() << "external selection is destroyed.";
    external = nullptr;
}
