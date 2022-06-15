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

#include <QObject>

DDP_ORGANIZER_BEGIN_NAMESPACE
class FileProxyModel;
class CanvasModelShell;
class OrganizerCreator
{
public:
    enum Mode {
        kNormalized = 0,
        kCustom
    };
    static class CanvasOrganizer *createOrganizer(Mode mode);
};

class CanvasOrganizer : public QObject
{
    Q_OBJECT
public:
    explicit CanvasOrganizer(QObject *parent = nullptr);
    virtual int mode() const = 0;
    virtual bool initialize(FileProxyModel *) = 0;
    virtual void setCanvasShell(CanvasModelShell *sh);
    virtual void setSurface(QWidget *w);
signals:

public slots:
protected slots:
    virtual bool filterDataRested(QList<QUrl> *urls);
    virtual bool filterDataInserted(const QUrl &url);
    virtual bool filterDataRenamed(const QUrl &oldUrl, const QUrl &newUrl);
protected:
    FileProxyModel *model = nullptr;
    CanvasModelShell *shell = nullptr;
    QWidget *surface = nullptr;
};

DDP_ORGANIZER_END_NAMESPACE

#endif // MUSTERMODE_H
