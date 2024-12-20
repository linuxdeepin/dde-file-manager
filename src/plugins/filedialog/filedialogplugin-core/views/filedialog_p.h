// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEDIALOG_P_H
#define FILEDIALOG_P_H

#include "filedialogplugin_core_global.h"
#include "filedialogstatusbar.h"
#include <dfm-base/interfaces/abstractframe.h>

#include <QObject>
#include <QEventLoop>
#include <QFileDialog>

#include <linux/limits.h>
#include <mutex>

DFMBASE_USE_NAMESPACE

namespace filedialog_core {

class FileDialog;
class FileDialogPrivate : public QObject
{
    Q_OBJECT
    friend class FileDialog;
    FileDialog *const q;

public:
    explicit FileDialogPrivate(FileDialog *qq);
    ~FileDialogPrivate();

    void handleSaveAcceptBtnClicked();
    void handleOpenAcceptBtnClicked();
    void handleOpenNewWindow(const QUrl &url);
    bool checkFileSuffix(const QString &filename, QString &suffix);

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
    QDir::Filters filters { QDir::NoFilter };
    int currentNameFilterIndex { -1 };
    QString currentInputName;
    bool allowMixedSelection { false };
    QFileDialog::Options options;
    QUrl currentUrl;
    QUrl lastVisitedDir;
};

}

#endif   // FILEDIALOG_P_H
