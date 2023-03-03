// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SURFACE_H
#define SURFACE_H

#include "ddplugin_organizer_global.h"

#include <QWidget>

namespace ddplugin_organizer {

class Surface : public QWidget
{
    Q_OBJECT
public:
    explicit Surface(QWidget *parent = nullptr);

signals:

public slots:
protected:
#ifdef QT_DEBUG
    void paintEvent(QPaintEvent *) override;
#endif
};

typedef QSharedPointer<Surface> SurfacePointer;

}

#endif // SURFACE_H
