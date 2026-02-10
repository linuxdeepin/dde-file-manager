// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TRASHCOREEVENTSENDER_H
#define TRASHCOREEVENTSENDER_H

#include "dfmplugin_trashcore_global.h"

#include <dfm-base/dfm_base_global.h>

#include <QObject>
#include <QSharedPointer>
#include <QTimer>

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
    enum class TrashState {
        Unknown,    // Initial state, not yet determined
        Empty,      // Trash is empty
        NotEmpty    // Trash contains files
    };

    explicit TrashCoreEventSender(QObject *parent = nullptr);
    void initTrashWatcher();
    bool checkAndStartWatcher();

private:
    QSharedPointer<DFMBASE_NAMESPACE::AbstractFileWatcher> trashFileWatcher = nullptr;
    TrashState trashState { TrashState::Unknown };
    QTimer timer;
};

}

#endif   // TRASHCOREEVENTSENDER_H
