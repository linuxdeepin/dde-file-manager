// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TRASHCOREEVENTSENDER_H
#define TRASHCOREEVENTSENDER_H

#include "dfmplugin_trashcore_global.h"

#include <dfm-base/dfm_base_global.h>

#include <QObject>
#include <QSharedPointer>

namespace dfmbase {
class AbstractFileWatcher;
}

namespace dfmplugin_trashcore {

class TrashCoreEventSender final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(TrashCoreEventSender)

public:
    static TrashCoreEventSender *instance();

private slots:
    void sendTrashStateChangedDel();
    void sendTrashStateChangedAdd();

private:
    explicit TrashCoreEventSender(QObject *parent = nullptr);
    void initTrashWatcher();

private:
    QSharedPointer<DFMBASE_NAMESPACE::AbstractFileWatcher> trashFileWatcher = nullptr;
    bool isEmpty { false };
};

}

#endif   // TRASHCOREEVENTSENDER_H
