// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EMBLEMEVENTRECEVIER_H
#define EMBLEMEVENTRECEVIER_H

#include "dfmplugin_emblem_global.h"
#include <dfm-base/interfaces/fileinfo.h>

#include <QObject>
#include <QPainter>

DPEMBLEM_BEGIN_NAMESPACE

class EmblemEventRecevier : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(EmblemEventRecevier)
public:
    static EmblemEventRecevier *instance();

    bool handlePaintEmblems(QPainter *painter, const QRectF &paintArea, const FileInfoPointer &info);

    void initializeConnections() const;

private:
    explicit EmblemEventRecevier(QObject *parent = nullptr);
};

DPEMBLEM_END_NAMESPACE

Q_DECLARE_METATYPE(QPainter *)

#endif   // EMBLEMEVENTRECEVIER_H
