/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liqiang<liqianga@uniontech.com>
 *
 * Maintainer: liqiang<liqianga@uniontech.com>
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

#include "screen/screenservice.h"
#include "services/backgroundservice.h"
#include "dfm-base/widgets/abstractcanvasmanager.h"
#include "dfm-base/widgets/abstractcanvasmodel.h"
#include "dfm-base/widgets/abstractcanvasselectionmodel.h"

#include <QObject>

DFMBASE_USE_NAMESPACE
DSB_D_BEGIN_NAMESPACE

class AbstractCanvas;
class CanvasViewManagerPrivate;
class CanvasViewManager : public dfmbase::AbstractCanvasManager
{
    Q_OBJECT
    friend class CanvasViewManagerPrivate;

public:
    explicit CanvasViewManager(QObject *parent = nullptr);
    ~CanvasViewManager() {}

private slots:
    void onFileLoadFinish();

private:
    void initConnect();
    void loadDataAndShow();

signals:

public slots:
    void onCanvasViewBuild();

private:
    CanvasViewManagerPrivate *const d { nullptr };
};

DSB_D_END_NAMESPACE
#endif   // CANVASVIEWMANAGER_H
