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
#include "emblemmanager.h"
#include "emblemhelper.h"

#include "dfm-base/interfaces/abstractfileinfo.h"
#include "dfm-base/base/schemefactory.h"

DFMBASE_USE_NAMESPACE
DPEMBLEM_USE_NAMESPACE

EmblemManager::EmblemManager(QObject *parent)
    : QObject(parent),
      helper(new EmblemHelper(this))
{
}

EmblemManager *EmblemManager::instance()
{
    static EmblemManager ins;
    return &ins;
}

void EmblemManager::paintEmblems(QPainter *painter, const QRectF &paintArea, const QUrl &url) const
{
    const QList<QRectF> &paintRects = caculatePaintRectList(paintArea);
    const QList<QIcon> &emblems = fetchEmblems(url);

    for (int i = 0; i < qMin(paintRects.count(), emblems.count()); ++i) {
        if (emblems.at(i).isNull())
            continue;

        emblems.at(i).paint(painter, paintRects.at(i).toRect());
    }
}

QList<QIcon> EmblemManager::fetchEmblems(const QUrl &url) const
{
    AbstractFileInfoPointer info = InfoFactory::create<AbstractFileInfo>(url);
    if (!info)
        return {};

    QList<QIcon> emblemList;

    emblemList = helper->getSystemEmblems(info);

    QMap<int, QIcon> gioEmblemsMap = helper->getGioEmblems(info);

    QMap<int, QIcon>::const_iterator iter = gioEmblemsMap.begin();
    while (iter != gioEmblemsMap.end()) {
        if (iter.key() == emblemList.count()) {
            emblemList.append(iter.value());
            ++iter;
            continue;
        }

        if (iter.key() > emblemList.count()) {
            emblemList.append(QIcon());
            continue;
        }

        if (emblemList.at(iter.key()).isNull())
            emblemList.replace(iter.key(), iter.value());

        ++iter;
    }

    return emblemList;
}

QList<QRectF> EmblemManager::caculatePaintRectList(const QRectF &paintArea) const
{
    QSizeF baseEmblemSize = paintArea.size() / 3;
    baseEmblemSize.setWidth(qBound(kMinEmblemSize, baseEmblemSize.width(), kMaxEmblemSize));
    baseEmblemSize.setHeight(qBound(kMinEmblemSize, baseEmblemSize.width(), kMaxEmblemSize));

    double offset = paintArea.width() / 8;
    const QSizeF &offsetSize = baseEmblemSize / 2;

    QList<QRectF> list;
    list.append(QRectF(QPointF(paintArea.right() - offset - offsetSize.width(),
                               paintArea.bottom() - offset - offsetSize.height()),
                       baseEmblemSize));
    list.append(QRectF(QPointF(paintArea.left() + offset - offsetSize.width(), list.first().top()), baseEmblemSize));
    list.append(QRectF(QPointF(list.at(1).left(), paintArea.top() + offset - offsetSize.height()), baseEmblemSize));
    list.append(QRectF(QPointF(list.first().left(), list.at(2).top()), baseEmblemSize));

    return list;
}
