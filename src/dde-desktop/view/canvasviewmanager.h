/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
 *             wangchunlin<wangchunlin@uniontech.com>
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

#ifndef CANVASVIEWMANAGER_H
#define CANVASVIEWMANAGER_H

#include "backgroundmanager.h"
#include "canvasgridview.h"
#include <QObject>

typedef QSharedPointer<CanvasGridView> CanvasViewPointer;

class CanvasViewManager : public QObject
{
    Q_OBJECT
public:
    explicit CanvasViewManager(BackgroundManager *bmrg,QObject *parent = nullptr);
    ~CanvasViewManager();
    inline QMap<ScreenPointer, CanvasViewPointer> canvas() const{return m_canvasMap;}
signals:
public slots:
    void onCanvasViewBuild(int mode);
private slots:
    void onBackgroundEnableChanged();
    void onScreenGeometryChanged();
    void onSyncOperation(int so, QVariant var);
    void onSyncSelection(CanvasGridView *v, DUrlList selected);
private:
    void init();
    void arrageEditDeal(const QString &);
private:
    BackgroundManager *m_background = nullptr;
    QMap<ScreenPointer, CanvasViewPointer> m_canvasMap;
};

#endif // CANVASVIEWMANAGER_H
