// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CANVASMANAGERSHELL_H
#define CANVASMANAGERSHELL_H

#include "ddplugin_organizer_global.h"

#include <QObject>

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
};

}

#endif // CANVASMANAGERSHELL_H
