/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuyangming<liuyangming@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             max-lv<lvwujun@uniontech.com>
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
#ifndef EMBLEMEVENTSEQUENCE_H
#define EMBLEMEVENTSEQUENCE_H

#include "dfmplugin_emblem_global.h"

#include "dfm-base/dfm_global_defines.h"
#include <dfm-framework/framework.h>

DPEMBLEM_BEGIN_NAMESPACE

class EmblemEventSequence : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(EmblemEventSequence)

public:
    static EmblemEventSequence *instance();

    bool doFetchExtendEmblems(const QUrl &url, QList<QIcon> *emblems);
    bool doFetchCustomEmblems(const QUrl &url, QList<QIcon> *emblems);

private:
    explicit EmblemEventSequence(QObject *parent = nullptr);
};

DPEMBLEM_END_NAMESPACE

#endif   // EMBLEMEVENTSEQUENCE_H
