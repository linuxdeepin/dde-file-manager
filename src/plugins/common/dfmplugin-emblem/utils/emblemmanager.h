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
#ifndef EMBLEMMANAGER_H
#define EMBLEMMANAGER_H

#include "dfmplugin_emblem_global.h"

#include "dfm-base/dfm_global_defines.h"

#include <QIcon>

DPEMBLEM_BEGIN_NAMESPACE

class EmblemHelper;
class EmblemManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(EmblemManager)
public:
    static EmblemManager *instance();

    bool paintEmblems(int role, const QUrl &url, QPainter *painter, QRectF *paintArea);

private:
    explicit EmblemManager(QObject *parent = nullptr);

    QList<QIcon> fetchEmblems(const QUrl &url) const;
    QList<QRectF> caculatePaintRectList(const QRectF &paintArea) const;

    EmblemHelper *helper { nullptr };
};

DPEMBLEM_END_NAMESPACE

#endif   // EMBLEMMANAGER_H
