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
#ifndef FRAMEMANAGER_P_H
#define FRAMEANAGER_P_H

#include "ddplugin_organizer_global.h"
#include "framemanager.h"
#include "surface.h"
#include "mode/canvasorganizer.h"
#include "models/collectionmodel.h"
#include "interface/canvasinterface.h"
#include "options/optionswindow.h"
#include "utils/organizerutils.h"

#include <dfm-framework/dpf.h>

namespace ddplugin_organizer {

class FrameManagerPrivate : public QObject
{
    Q_OBJECT
public:
    explicit FrameManagerPrivate(FrameManager *qq);
    ~FrameManagerPrivate() override;
    void buildSurface();
    void clearSurface();
    SurfacePointer createSurface(QWidget *root);
    void layoutSurface(QWidget *root, SurfacePointer surface, bool hidden = false);
    void buildOrganizer();
public slots:
    void refeshCanvas();
public slots:
    void enableChanged(bool e);
    void switchToCustom();
    void switchToNormalized(int cf);
    void displaySizeChanged(int size);
    void showOptionWindow();
public slots:
    bool filterShortcutkeyPress(int viewIndex, int key, int modifiers) const;
    bool filterWheel(int viewIndex, const QPoint &angleDelta, bool ctrl) const;
protected:
    QWidget *findView(QWidget *root) const;
public:
    QMap<QString, SurfacePointer> surfaceWidgets;
    CanvasOrganizer *organizer = nullptr;
    CollectionModel *model = nullptr;
    CanvasInterface *canvas = nullptr;
    OptionsWindow *options = nullptr;
private:
    FrameManager *q;
};

}

#endif // FRAMEMANAGER_P_H
