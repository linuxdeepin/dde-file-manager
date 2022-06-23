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
#ifndef MUSTERMODE_H
#define MUSTERMODE_H

#include "ddplugin_organizer_global.h"
#include "organizer_defines.h"

#include <QObject>

class QMimeData;

DDP_ORGANIZER_BEGIN_NAMESPACE
class FileProxyModel;
class CanvasModelShell;
class CanvasViewShell;
class CanvasGridShell;
class OrganizerCreator
{
public:
    static class CanvasOrganizer *createOrganizer(OrganizerMode mode);
};

class CanvasOrganizer : public QObject
{
    Q_OBJECT
public:
    explicit CanvasOrganizer(QObject *parent = nullptr);
    ~CanvasOrganizer() override;
    virtual OrganizerMode mode() const = 0;
    virtual bool initialize(FileProxyModel *) = 0;
    virtual void setCanvasModelShell(CanvasModelShell *sh);
    virtual void setCanvasViewShell(CanvasViewShell *sh);
    virtual void setCanvasGridShell(CanvasGridShell *sh);
    virtual void setSurface(QWidget *w);
    virtual void reset();

signals:
    void collectionChanged();

protected slots:
    virtual bool filterDataRested(QList<QUrl> *urls);
    virtual bool filterDataInserted(const QUrl &url);
    virtual bool filterDataRenamed(const QUrl &oldUrl, const QUrl &newUrl);
    virtual bool filterDropData(int viewIndex, const QMimeData *mimeData, const QPoint &viewPoint);
protected:
    FileProxyModel *model = nullptr;
    CanvasModelShell *canvasModelShell = nullptr;
    CanvasViewShell *canvasViewShell = nullptr;
    CanvasGridShell *canvasGridShell = nullptr;
    QWidget *surface = nullptr;
};

DDP_ORGANIZER_END_NAMESPACE

#endif // MUSTERMODE_H
