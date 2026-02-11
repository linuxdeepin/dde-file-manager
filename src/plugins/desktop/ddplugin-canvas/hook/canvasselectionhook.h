// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CANVASSELECTIONHOOK_H
#define CANVASSELECTIONHOOK_H

#include "model/selectionhookinterface.h"

#include <QObject>

namespace ddplugin_canvas {
class CanvasSelectionHook : public QObject, public SelectionHookInterface
{
    Q_OBJECT
public:
    explicit CanvasSelectionHook(QObject *parent = nullptr);
    // signals
    void clear() const override;
};

}
#endif // CANVASSELECTIONHOOK_H
