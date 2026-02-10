// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MODELHOOKINTERFACE_H
#define MODELHOOKINTERFACE_H

#include "ddplugin_canvas_global.h"

#include <QObject>

class QMimeData;

namespace ddplugin_canvas {

class ModelHookInterface
{
public:
    explicit ModelHookInterface();
    virtual ~ModelHookInterface();
public:
    //! the \a userData must be pointer of QVariantHash
    virtual bool modelData(const QUrl &url, int role, QVariant *out, void *extData = nullptr) const;
    virtual bool dataInserted(const QUrl &url, void *extData = nullptr) const;
    virtual bool dataRemoved(const QUrl &url, void *extData = nullptr) const;    // must return false
    virtual bool dataRenamed(const QUrl &oldUrl, const QUrl &newUrl, void *extData = nullptr) const;
    virtual bool dataRested(QList<QUrl> *urls, void *extData = nullptr) const;   // must return false
    virtual bool dataChanged(const QUrl &url, const QVector<int> &roles, void *extData = nullptr) const;   // must return false
    virtual bool dropMimeData(const QMimeData *data, const QUrl &dir, Qt::DropAction action, void *extData = nullptr) const;
    virtual bool mimeData(const QList<QUrl> &urls, QMimeData *out, void *extData = nullptr) const;
    virtual bool mimeTypes (QStringList *types, void *extData = nullptr) const;
    virtual bool sortData (int role, int order, QList<QUrl> *files, void *extData = nullptr) const; // 待定，还需考虑ui交互问题

    //signals
    virtual void hiddenFlagChanged(bool hidden) const;
};

}

#endif // MODELHOOKINTERFACE_H
