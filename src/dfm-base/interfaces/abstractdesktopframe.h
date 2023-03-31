// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ABSTRACTDESKTOPFRAME_H
#define ABSTRACTDESKTOPFRAME_H

#include <dfm-base/dfm_base_global.h>
#include <dfm-base/dfm_desktop_defines.h>

#include <QObject>

namespace dfmbase {

class AbstractDesktopFrame : public QObject
{
    Q_OBJECT
public:
    explicit AbstractDesktopFrame(QObject *parent = nullptr);
    virtual QList<QWidget *> rootWindows() const = 0;
    virtual void layoutChildren() = 0;
Q_SIGNALS:   // must connect with Qt::DirectConnection
    void windowAboutToBeBuilded();
    void windowBuilded();
    void windowShowed();
    void geometryChanged();
    void availableGeometryChanged();
public Q_SLOTS:
};

}

#endif   // ABSTRACTDESKTOPFRAME_H
