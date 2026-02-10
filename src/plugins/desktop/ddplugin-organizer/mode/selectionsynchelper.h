// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef SELECTIONSYNCHELPER_H
#define SELECTIONSYNCHELPER_H

#include "models/itemselectionmodel.h"
#include "interface/canvasselectionshell.h"

#include <QObject>

namespace ddplugin_organizer {

class SelectionSyncHelper : public QObject
{
    Q_OBJECT
public:
    explicit SelectionSyncHelper(QObject *parent = nullptr);
    void setInnerModel(ItemSelectionModel *sel);
    void setExternalModel(QItemSelectionModel *sel);
    void setShell(CanvasSelectionShell *sh);
    inline void setEnabled(bool en) {
        enabled = en;
    }
signals:

public slots:
    void clearExteralSelection();
    void clearInnerSelection();
private slots:
    void innerModelDestroyed();
    void externalModelDestroyed();
protected:
    bool enabled = false;
    CanvasSelectionShell *shell = nullptr;
    ItemSelectionModel *inner = nullptr;
    QItemSelectionModel *external = nullptr;

};
}
#endif // SELECTIONSYNCHELPER_H
