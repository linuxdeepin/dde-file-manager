// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MODELDATAHANDLER_H
#define MODELDATAHANDLER_H

#include "ddplugin_organizer_global.h"

#include <QObject>
#include <QUrl>
#include <QVector>

namespace ddplugin_organizer {

class ModelDataHandler
{
    Q_DISABLE_COPY(ModelDataHandler)
public:
    explicit ModelDataHandler();
    virtual ~ModelDataHandler();
    virtual bool acceptInsert(const QUrl &url);
    virtual QList<QUrl> acceptReset(const QList<QUrl> &urls);
    virtual bool acceptRename(const QUrl &oldUrl, const QUrl &newUrl);
    virtual bool acceptUpdate(const QUrl &url, const QVector<int> &roles = {});
};
}

#endif // MODELDATAHANDLER_H
