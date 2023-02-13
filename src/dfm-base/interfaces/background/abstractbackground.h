// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ABSTRACTBACKGROUND_H
#define ABSTRACTBACKGROUND_H

#include "dfm-base/dfm_base_global.h"

#include <QWidget>

namespace dfmbase {
class AbstractBackground : public QWidget
{
    Q_OBJECT
public:
    enum Mode {
        Statics,
        Dynamic,
        ModeCustom = 10,
    };

    enum Zoom {
        Tile,
        Stretch,
        Fit,
        Fill,
        Center,
        ZoomCustom = 50,
    };
    explicit AbstractBackground(const QString &screenName, QWidget *parent = nullptr);
    virtual void setMode(int mode);
    virtual int mode() const;
    virtual void setZoom(int zoom);
    virtual int zoom() const;
    virtual void setDisplay(const QString &path);
    virtual QString display() const;
    virtual void updateDisplay();
protected:
    int displayMode = Mode::Statics;
    int displayZoom = Zoom::Stretch;
    QString screen;
    QString filePath;
};

typedef QSharedPointer<AbstractBackground> BackgroundWidgetPointer;

}

#endif // ABSTRACTBACKGROUND_H
