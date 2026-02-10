// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ABSTRACTSCREEN_H
#define ABSTRACTSCREEN_H

#include <dfm-base/dfm_base_global.h>

#include <QObject>
#include <QRect>

namespace dfmbase {
class AbstractScreen : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(AbstractScreen)
public:
    explicit AbstractScreen(QObject *parent = nullptr);
    virtual QString name() const = 0;
    virtual QRect geometry() const = 0;
    virtual QRect availableGeometry() const = 0;
    virtual QRect handleGeometry() const = 0;
Q_SIGNALS:
    void geometryChanged(const QRect &);
    void availableGeometryChanged(const QRect &);
};

typedef QSharedPointer<AbstractScreen> ScreenPointer;

}

#endif   // ABSTRACTSCREEN_H
