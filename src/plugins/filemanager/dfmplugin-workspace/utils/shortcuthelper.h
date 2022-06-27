/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: liuyangming<liuyangming@uniontech.com>
 *             gongheng<gongheng@uniontech.com>
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
#ifndef SHORTCUTHELPER_H
#define SHORTCUTHELPER_H

#include "dfmplugin_workspace_global.h"

#include <QObject>
#include <QKeySequence>
#include <QKeyEvent>

namespace dfmplugin_workspace {

class FileView;
class ShortcutHelper : public QObject
{
    Q_OBJECT
public:
    explicit ShortcutHelper(FileView *parent);
    void registerShortcut();
    void registerAction(QKeySequence::StandardKey shortcut, bool autoRepeat = true);
    bool processKeyPressEvent(QKeyEvent *event);

protected slots:
    void acitonTriggered();
    void copyFiles();
    void cutFiles();
    void pasteFiles();
    void undoFiles();
    void deleteFiles();
    void moveToTrash();
    void touchFolder();
    void toggleHiddenFiles();
    void showFilesProperty();
    void previewFiles();
    void openAction(const QList<QUrl> &urls, const DirOpenMode openMode = DirOpenMode::kOpenInCurrentWindow);
    void openInTerminal();
    void cdUp();

private:
    bool normalKeyPressEventHandle(const QKeyEvent *event);

private:
    FileView *view { nullptr };
};

}

#endif   // SHORTCUTHELPER_H
