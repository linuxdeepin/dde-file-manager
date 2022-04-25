/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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
#ifndef CANVASMODELEXTEND_H
#define CANVASMODELEXTEND_H

#include "ddplugin_canvas_global.h"
#include "model/modelextendinterface.h"

#include <QObject>

DDP_CANVAS_BEGIN_NAMESPACE

class CanvasModelExtendPrivate;
class CanvasModelExtend : public QObject, public ModelExtendInterface
{
    Q_OBJECT
    friend class CanvasModelExtendPrivate;
public:
    explicit CanvasModelExtend(QObject *parent = nullptr);
    bool init();
    bool modelData(const QUrl &url, int role, QVariant *out, void *userData = nullptr) const override;
signals:

public slots:
private:
    CanvasModelExtendPrivate *d;
};

DDP_CANVAS_END_NAMESPACE

#endif // CANVASMODELEXTEND_H
