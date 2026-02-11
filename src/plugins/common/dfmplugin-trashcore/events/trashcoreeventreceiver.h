// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TRASHCOREEVENTRECEIVER_H
#define TRASHCOREEVENTRECEIVER_H

#include "dfmplugin_trashcore_global.h"

#include <dfm-base/interfaces/abstractjobhandler.h>

#include <QObject>
#include <QUrl>

namespace dfmplugin_trashcore {

class TrashCoreEventReceiver final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(TrashCoreEventReceiver)

public:
    static TrashCoreEventReceiver *instance();

public slots:
    void handleEmptyTrash(const quint64 windowId = 0);
    bool cutFileFromTrash(const quint64 windowId, const QList<QUrl> sources,
                          const QUrl target, const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags);
    bool copyFromFile(const quint64 windowId, const QList<QUrl> sources, const QUrl target, const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags);

private:
    explicit TrashCoreEventReceiver(QObject *parent = nullptr);
};

}

#endif   // COREEVENTRECEIVER_H
