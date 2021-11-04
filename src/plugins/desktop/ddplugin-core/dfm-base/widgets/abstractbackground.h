/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huangyu<huangyub@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
 *             zhangyu<zhangyub@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef ABSTRACTBACKGROUND_H
#define ABSTRACTBACKGROUND_H

#include "dfm-base/dfm_base_global.h"

#include <QWidget>

DFMBASE_BEGIN_NAMESPACE
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
    explicit AbstractBackground(QWidget *parent = nullptr);
    virtual void setMode(int mode) {displayMode = static_cast<Mode>(mode);}
    virtual int mode() const {return displayMode;}
    virtual void setZoom(int zoom) {displayZoom = static_cast<Mode>(zoom);}
    virtual int zoom() const {return displayZoom;}
    virtual void setDisplay(const QString &path) {filePath = path;}
    virtual QString display() const {return filePath;}
protected:
    int displayMode = Mode::Statics;
    int displayZoom = Zoom::Fit;
    QString filePath = "";
};

DFMBASE_END_NAMESPACE

#endif // ABSTRACTBACKGROUND_H
