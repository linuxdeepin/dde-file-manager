/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuyangming<liuyangming@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#ifndef EMBLEMEVENTRECEVIER_H
#define EMBLEMEVENTRECEVIER_H

#include "dfmplugin_emblem_global.h"

#include <QObject>
#include <QPainter>

DPEMBLEM_BEGIN_NAMESPACE

class EmblemEventRecevier : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(EmblemEventRecevier)
public:
    static EmblemEventRecevier *instance();

    void handlePaintEmblems(QPainter *painter, const QRectF &paintArea, const QUrl &url);

    void initializeConnections() const;

private:
    explicit EmblemEventRecevier(QObject *parent = nullptr);
};

DPEMBLEM_END_NAMESPACE

Q_DECLARE_METATYPE(QPainter *)

#endif   // EMBLEMEVENTRECEVIER_H
