// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CANVASSELECTIONSHELL_H
#define CANVASSELECTIONSHELL_H

#include "ddplugin_organizer_global.h"

#include <QObject>

class QItemSelectionModel;

namespace ddplugin_organizer {

class CanvasSelectionShell : public QObject
{
    Q_OBJECT
public:
    explicit CanvasSelectionShell(QObject *parent = nullptr);
    ~CanvasSelectionShell();
    bool initialize();
    QItemSelectionModel *selectionModel();
signals:
    void requestClear();
public slots:
private slots:
    void eventClear();
};
}

#endif // CANVASSELECTIONSHELL_H
