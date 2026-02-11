// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EMBLEMEVENTSEQUENCE_H
#define EMBLEMEVENTSEQUENCE_H

#include "dfmplugin_emblem_global.h"

#include <dfm-base/dfm_global_defines.h>

#include <dfm-framework/dpf.h>

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
