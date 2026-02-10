// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EMBLEMMANAGER_H
#define EMBLEMMANAGER_H

#include "dfmplugin_emblem_global.h"

#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/interfaces/fileinfo.h>

#include <QIcon>

DPEMBLEM_BEGIN_NAMESPACE

class EmblemHelper;
class EmblemManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(EmblemManager)
public:
    static EmblemManager *instance();

    bool paintEmblems(int role, const FileInfoPointer &info, QPainter *painter, QRectF *paintArea);

private:
    explicit EmblemManager(QObject *parent = nullptr);

    EmblemHelper *helper { nullptr };
};

DPEMBLEM_END_NAMESPACE

#endif   // EMBLEMMANAGER_H
