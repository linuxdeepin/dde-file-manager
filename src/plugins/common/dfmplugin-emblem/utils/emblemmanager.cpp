// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "emblemmanager.h"

#include "utils/emblemhelper.h"
#include "events/emblemeventsequence.h"

#include "dfm-base/interfaces/abstractfileinfo.h"
#include "dfm-base/base/schemefactory.h"

#include <QPainter>

DFMBASE_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE
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

bool EmblemManager::paintEmblems(int role, const QUrl &url, QPainter *painter, QRectF *paintArea)
{
    Q_ASSERT(qApp->thread() == QThread::currentThread());
    Q_ASSERT(painter);
    Q_ASSERT(paintArea);
    if (role != kItemIconRole || !url.isValid())
        return false;

    // for produce
    helper->pending(url);

    // consume
    QList<QIcon> emblems { helper->emblemIcons(url) };

    // add custom emblem icons
    EmblemEventSequence::instance()->doFetchCustomEmblems(url, &emblems);
    // add extension lib emblem icons
    EmblemEventSequence::instance()->doFetchExtendEmblems(url, &emblems);

    if (emblems.isEmpty())
        return false;

    const QList<QRectF> &paintRects = helper->emblemRects(*paintArea);
    for (int i = 0; i < qMin(paintRects.count(), emblems.count()); ++i) {
        if (emblems.at(i).isNull())
            continue;
        emblems.at(i).paint(painter, paintRects.at(i).toRect());
    }

    return false;
}
