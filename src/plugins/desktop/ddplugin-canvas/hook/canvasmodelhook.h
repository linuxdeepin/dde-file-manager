// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CANVASMODELHOOK_H
#define CANVASMODELHOOK_H

#include "ddplugin_canvas_global.h"
#include "model/modelhookinterface.h"

#include <QObject>

namespace ddplugin_canvas {

class CanvasModelHook : public QObject, public ModelHookInterface
{
    Q_OBJECT
public:
    explicit CanvasModelHook(QObject *parent = nullptr);
    bool modelData(const QUrl &url, int role, QVariant *out, void *extData = nullptr) const override;
    bool dataInserted(const QUrl &url, void *extData = nullptr) const override;
    bool dataRemoved(const QUrl &url, void *extData = nullptr) const override;    // must return false
    bool dataRenamed(const QUrl &oldUrl, const QUrl &newUrl, void *extData = nullptr) const override;
    bool dataRested(QList<QUrl> *urls, void *extData = nullptr) const override;   // must return false
    bool dataChanged(const QUrl &url, const QVector<int> &roles, void *extData = nullptr) const override;   // must return false
    bool dropMimeData(const QMimeData *data, const QUrl &dir, Qt::DropAction action, void *extData = nullptr) const override;
    bool mimeData(const QList<QUrl> &urls, QMimeData *out, void *extData = nullptr) const override;
    bool mimeTypes (QStringList *types, void *extData = nullptr) const override;
    bool sortData (int role, int order, QList<QUrl> *files, void *extData = nullptr) const override; // 待定，还需考虑ui交互问题

    void hiddenFlagChanged(bool show) const override;
};

}

#endif // CANVASMODELHOOK_H
