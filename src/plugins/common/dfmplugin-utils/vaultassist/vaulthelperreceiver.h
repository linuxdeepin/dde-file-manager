// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VAULTHELPERRECEIVER_H
#define VAULTHELPERRECEIVER_H

#include "dfmplugin_utils_global.h"
#include <dfm-base/interfaces/abstractjobhandler.h>

#include <QObject>

namespace dfmplugin_utils {
class VaultHelperReceiver : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(VaultHelperReceiver)

public:
    explicit VaultHelperReceiver(QObject *parent = Q_NULLPTR);
    void initEventConnect();
    void callBackFunction(const DFMBASE_NAMESPACE::AbstractJobHandler::CallbackArgus args);

public slots:
    bool handlemoveToTrash(const quint64 windowId, const QList<QUrl> &sources,
                           const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags);
    void handleFinishedNotify(const JobInfoPointer &jobInfo);
};

}
#endif // VAULTHELPERRECEIVER_H
