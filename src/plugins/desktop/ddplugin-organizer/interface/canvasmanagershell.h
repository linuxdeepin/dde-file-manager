// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CANVASMANAGERSHELL_H
#define CANVASMANAGERSHELL_H

#include "ddplugin_organizer_global.h"

#include <QObject>

class QItemSelectionModel;
namespace ddplugin_organizer {

class CanvasManagerShell : public QObject
{
    Q_OBJECT
public:
    explicit CanvasManagerShell(QObject *parent = nullptr);
    ~CanvasManagerShell();
    bool initialize();

public:
    int iconLevel() const;
    void setIconLevel(const int level);
signals:
    void iconSizeChanged(const int level);
    void fontChanged();
    void requestRefresh(bool silence);
};

}

#endif   // CANVASMANAGERSHELL_H
