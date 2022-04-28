/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#ifndef FILEDIALOG_P_H
#define FILEDIALOG_P_H

#include "filedialogplugin_core_global.h"
#include "filedialogstatusbar.h"
#include "dfm-base/interfaces/abstractframe.h"

#include <QObject>
#include <QEventLoop>
#include <QFileDialog>

#include <linux/limits.h>
#include <mutex>

DFMBASE_USE_NAMESPACE

DIALOGCORE_BEGIN_NAMESPACE

class FileDialog;
class FileDialogPrivate : public QObject
{
    Q_OBJECT
    friend class FileDialog;
    FileDialog *const q;

public:
    explicit FileDialogPrivate(FileDialog *qq);

    void handleSaveAcceptBtnClicked();
    void handleOpenAcceptBtnClicked();

private:
    static constexpr int kDefaultWindowWidth { 960 };
    static constexpr int kDefaultWindowHeight { 540 };

    bool isFileView { false };
    bool lastIsFileView { false };
    bool hideOnAccept { true };
    FileDialogStatusBar *statusBar { nullptr };
    QEventLoop *eventLoop { nullptr };
    QFileDialog::FileMode fileMode { QFileDialog::AnyFile };
    QFileDialog::AcceptMode acceptMode { QFileDialog::AcceptOpen };
    bool acceptCanOpenOnSave { false };
    QStringList nameFilters;
    QDir::Filters filters { nullptr };
    int currentNameFilterIndex { -1 };
    QString currentInputName;
    bool allowMixedSelection { false };
    QFileDialog::Options options;
};

DIALOGCORE_END_NAMESPACE

#endif   // FILEDIALOG_P_H
