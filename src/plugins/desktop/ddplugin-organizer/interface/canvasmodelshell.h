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
#ifndef CANVASMODELSHELL_H
#define CANVASMODELSHELL_H

#include "ddplugin_organizer_global.h"

#include <QObject>

class QMimeData;

DDP_ORGANIZER_BEGIN_NAMESPACE
class CanvasInterface;
class CanvasModelShell : public QObject
{
    Q_OBJECT
public:
    explicit CanvasModelShell(QObject *parent = nullptr);
    ~CanvasModelShell();
    bool initialize();
    void refresh(int ms = 0);
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

DDP_ORGANIZER_END_NAMESPACE


#endif // CANVASMODELSHELL_H
