// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SHORTCUTHELPER_H
#define SHORTCUTHELPER_H

#include "dfmplugin_workspace_global.h"

#include <dfm-base/utils/private/windowtransition_p.h>

#include <QObject>
#include <QKeySequence>
#include <QKeyEvent>
#include <QList>
#include <QPixmap>
#include <QRect>
#include <QSize>
#include <QUrl>

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

    bool reverseSelect();
    void renameProcessing();

protected slots:
    void acitonTriggered();
    void copyFiles();
    void copyFilePath();
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
    void redoFiles();

private:
    bool normalKeyPressEventHandle(const QKeyEvent *event);
    bool doEnterPressed();
    void initRenameProcessTimer();
    void launchPreview(const QList<QUrl> &selectUrls, const QList<QUrl> &currentDirUrls, const QUrl &anchorUrl);
    bool previewAnchor(const QUrl &url, QRect *anchor, QPixmap *snapshot) const;

private:
    FileView *view { nullptr };
    QTimer *renameProcessTimer { nullptr };
    bool enterTriggerFlag { false };
};

}

#endif   // SHORTCUTHELPER_H
