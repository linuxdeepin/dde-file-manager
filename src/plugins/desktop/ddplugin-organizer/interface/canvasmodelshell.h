// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CANVASMODELSHELL_H
#define CANVASMODELSHELL_H

#include "ddplugin_organizer_global.h"

#include <QObject>

class QMimeData;

namespace ddplugin_organizer {
class CanvasInterface;
class CanvasModelShell : public QObject
{
    Q_OBJECT
public:
    explicit CanvasModelShell(QObject *parent = nullptr);
    ~CanvasModelShell();
    bool initialize();
    void refresh(int ms = 0, bool updateFile = true);
    bool fetch(const QUrl &url);
    bool take(const QUrl &url);
signals: // unqiue and direct signals
    bool filterDataRested(QList<QUrl> *urls);
    bool filterDataInserted(const QUrl &url);
    bool filterDataRenamed(const QUrl &oldUrl, const QUrl &newUrl);
public slots:
private slots:
    bool eventDataRested(QList<QUrl> *urls, void *extData);
    bool eventDataInserted(const QUrl &url, void *extData);
    bool eventDataRenamed(const QUrl &oldUrl, const QUrl &newUrl, void *extData);
};

}


#endif // CANVASMODELSHELL_H
