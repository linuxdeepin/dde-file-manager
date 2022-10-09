/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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
    bool dataChanged(const QUrl &url, void *extData = nullptr) const override;   // must return false
    bool dropMimeData(const QMimeData *data, const QUrl &dir, Qt::DropAction action, void *extData = nullptr) const override;
    bool mimeData(const QList<QUrl> &urls, QMimeData *out, void *extData = nullptr) const override;
    bool mimeTypes (QStringList *types, void *extData = nullptr) const override;
    bool sortData (int role, int order, QList<QUrl> *files, void *extData = nullptr) const override; // 待定，还需考虑ui交互问题

    void hiddenFlagChanged(bool show) const override;
};

}

#endif // CANVASMODELHOOK_H
