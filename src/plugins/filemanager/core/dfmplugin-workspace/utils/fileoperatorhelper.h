// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEOPERATORHELPER_H
#define FILEOPERATORHELPER_H

#include "dfmplugin_workspace_global.h"
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/interfaces/abstractjobhandler.h>

#include <QObject>
#include <QUrl>

namespace dfmplugin_workspace {
class FileView;
class FileOperatorHelper : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(FileOperatorHelper)

public:
    static FileOperatorHelper *instance();
    void touchFolder(const FileView *view);
    void touchFiles(const FileView *view, const DFMGLOBAL_NAMESPACE::CreateFileType type, QString suffix = "");
    void touchFiles(const FileView *view, const QUrl &source);
    void openFiles(const FileView *view);
    void openFiles(quint64 winId, const QList<QUrl> &urls);
    void openFiles(const FileView *view, const QList<QUrl> &urls);
    void openFilesByMode(const FileView *view, const QList<QUrl> &urls, const DirOpenMode mode = DirOpenMode::kOpenInCurrentWindow);
    void openFilesByApp(const FileView *view, const QList<QUrl> &urls, const QList<QString> &apps);
    void renameFile(const FileView *view, const QUrl &oldUrl, const QUrl &newUrl);
    void copyFiles(const FileView *view);
    void cutFiles(const FileView *view);
    void pasteFiles(const FileView *view);
    void undoFiles(const FileView *view);
    void moveToTrash(const FileView *view);
    void moveToTrash(const FileView *view, const QList<QUrl> &urls);
    void deleteFiles(const FileView *view);
    void createSymlink(const FileView *view, QUrl targetParent = QUrl());
    void openInTerminal(const FileView *view);
    void showFilesProperty(const FileView *view);
    void sendBluetoothFiles(const FileView *view);
    void previewFiles(const FileView *view, const QList<QUrl> &selectUrls, const QList<QUrl> &currentDirUrls);
    void dropFiles(const FileView *view, const Qt::DropAction &action, const QUrl &targetUrl, const QList<QUrl> &urls);

    void renameFilesByReplace(const QWidget *sender, const QList<QUrl> &urlList, const QPair<QString, QString> &replacePair);
    void renameFilesByAdd(const QWidget *sender, const QList<QUrl> &urlList, const QPair<QString, DFMBASE_NAMESPACE::AbstractJobHandler::FileNameAddFlag> &addPair);
    void renameFilesByCustom(const QWidget *sender, const QList<QUrl> &urlList, const QPair<QString, QString> &customPair);
    void redoFiles(const FileView *view);

private:
    explicit FileOperatorHelper(QObject *parent = nullptr);
    void callBackFunction(const DFMBASE_NAMESPACE::AbstractJobHandler::CallbackArgus args);
    void undoCallBackFunction(QSharedPointer<DFMBASE_NAMESPACE::AbstractJobHandler> handler);

    DFMBASE_NAMESPACE::AbstractJobHandler::OperatorCallback callBack;
    DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback undoCallBack;
    DFMBASE_NAMESPACE::AbstractJobHandler::OperatorHandleCallback redoCallBack;
};

#define FileOperatorHelperIns            \
    using namespace dfmplugin_workspace; \
    ::FileOperatorHelper::instance()
}

#endif   // FILEOPERATORHELPER_H
