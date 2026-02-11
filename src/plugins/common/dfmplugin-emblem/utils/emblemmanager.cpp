// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "emblemmanager.h"

#include "utils/emblemhelper.h"
#include "events/emblemeventsequence.h"

#include <dfm-base/base/schemefactory.h>

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

bool EmblemManager::paintEmblems(int role, const FileInfoPointer &info, QPainter *painter, QRectF *paintArea)
{
    Q_ASSERT(qApp->thread() == QThread::currentThread());
    Q_ASSERT(painter);
    Q_ASSERT(paintArea);

    painter->setRenderHints(QPainter::SmoothPixmapTransform);

    if (role != kItemIconRole || info.isNull())
        return false;

    // add system emblem icons
    QList<QIcon> emblems { helper->systemEmblems(info) };

    //  only paitn system emblem icons if url is prohibited
    const QUrl &url = info->urlOf(UrlInfoType::kUrl);
    if (!helper->isExtEmblemProhibited(info, url)) {
        // add gio embelm icons
        helper->pending(info);
        const auto &gioEmblems = helper->gioEmblemIcons(url);
        if (emblems.isEmpty()) {
            emblems = gioEmblems;
        } else if (emblems.size() < gioEmblems.size()) {
            // Ensure that the custom emblems do not affect the display position by the system emblems
            emblems.append(gioEmblems.mid(emblems.size()));
        }

        // add custom emblem icons
        EmblemEventSequence::instance()->doFetchCustomEmblems(url, &emblems);
        // add extension lib emblem icons
        EmblemEventSequence::instance()->doFetchExtendEmblems(url, &emblems);
    }

    if (emblems.isEmpty())
        return false;

    const QList<QRectF> &paintRects = helper->emblemRects(*paintArea);
    for (int i = 0; i < qMin(paintRects.count(), emblems.count()); ++i) {
        if (emblems.at(i).isNull())
            continue;
        // NOTE: for some special icons, the QIcon::paint function will cast lots of cpu resource.
        // so use the painter drawPixmap function to paint the emblems.
        const QRect emblemRect = paintRects.at(i).toRect();
        if (emblemRect.isEmpty())
            continue;

        const qreal dpr = painter->device() ? painter->device()->devicePixelRatioF() : 1.0;
        const QSize deviceSize(qMax(1, qRound(emblemRect.width()  * dpr)),
                               qMax(1, qRound(emblemRect.height() * dpr)));

        QPixmap emblemPix = emblems.at(i).pixmap(deviceSize);
        emblemPix.setDevicePixelRatio(dpr);

        const qreal ax = qRound(emblemRect.x() * dpr) / dpr;
        const qreal ay = qRound(emblemRect.y() * dpr) / dpr;
        painter->drawPixmap(QPointF(ax, ay), emblemPix);
    }

    return true;
}
