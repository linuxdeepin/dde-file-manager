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
#ifndef SHORTCUTOPER_H
#define SHORTCUTOPER_H

#include "dfm_desktop_service_global.h"

#include <QObject>
#include <QKeyEvent>

DSB_D_BEGIN_NAMESPACE
class CanvasView;
class ShortcutOper : public QObject
{
    Q_OBJECT
public:
    explicit ShortcutOper(CanvasView *parent);
    void regShortcut();
    bool keyPressed(QKeyEvent *event);
    bool disableShortcut() const;
protected slots:
    void acitonTriggered();
    void helpAction();
    void openAction();
    void moveToTrash();
    void deleteFile();
    void copyFiles();
    void cutFiles();
    void pasteFiles();
    void tabToFirst();
    void showMenu();
    void clearClipBoard();
    void swichHidden();
    void showProperty();
protected:
    CanvasView *view;
};

DSB_D_END_NAMESPACE

#endif // SHORTCUTOPER_H
