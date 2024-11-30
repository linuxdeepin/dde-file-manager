// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
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
        int systemEmblemsSize = emblems.size();
        if (systemEmblemsSize == 0) {
            emblems.append(helper->gioEmblemIcons(url));
        } else if (systemEmblemsSize > 0 && systemEmblemsSize < 4) {
            const QList<QIcon> &gioIcons { helper->gioEmblemIcons(url) };
            int gioIconsSize { gioIcons.size() };
            if (gioIconsSize > systemEmblemsSize) {
                for (int i = systemEmblemsSize; i < gioIconsSize; ++i) {
                    emblems.append(gioIcons[i]);
                }
            }
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
        emblems.at(i).paint(painter, paintRects.at(i).toRect());
    }

    return true;
}
