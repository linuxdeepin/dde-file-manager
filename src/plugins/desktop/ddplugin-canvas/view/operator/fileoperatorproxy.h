// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEOPERATORPROXY_H
#define FILEOPERATORPROXY_H

#include "ddplugin_canvas_global.h"

#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/interfaces/abstractjobhandler.h>

#include <QObject>
#include <QPoint>

namespace ddplugin_canvas {
class CanvasView;
class FileOperatorProxyPrivate;
class FileOperatorProxy : public QObject
{
    Q_OBJECT
public:
    static FileOperatorProxy *instance();
    void touchFile(const CanvasView *view, const QPoint pos, const DFMBASE_NAMESPACE::Global::CreateFileType type, QString suffix = "");
    void touchFile(const CanvasView *view, const QPoint pos, const QUrl &source);
    void touchFolder(const CanvasView *view, const QPoint pos);
    void copyFiles(const CanvasView *view);
    void copyFilePath(const CanvasView *view);
    void cutFiles(const CanvasView *view);
    void pasteFiles(const CanvasView *view, const QPoint pos = QPoint(0, 0));
    void openFiles(const CanvasView *view);
    void openFiles(const CanvasView *view, const QList<QUrl> &urls);
    Q_INVOKABLE void renameFile(int wid, const QUrl &oldUrl, const QUrl &newUrl);
    void renameFiles(const CanvasView *view, const QList<QUrl> &urls, const QPair<QString, QString> &pair, const bool replace);
    void renameFiles(const CanvasView *view, const QList<QUrl> &urls, const QPair<QString, DFMBASE_NAMESPACE::AbstractJobHandler::FileNameAddFlag> pair);
    void openFilesByApp(const CanvasView *view);
    void moveToTrash(const CanvasView *view);
    void deleteFiles(const CanvasView *view);
    void showFilesProperty(const CanvasView *view);
    void sendFilesToBluetooth(const CanvasView *view);
    void undoFiles(const CanvasView *view);
    void redoFiles(const CanvasView *view);

    void dropFiles(const Qt::DropAction &action, const QUrl &targetUrl, const QList<QUrl> &urls);
    void dropToTrash(const QList<QUrl> &urls);
    void dropToApp(const QList<QUrl> &urls, const QString &app);

    Q_INVOKABLE QPair<QString, QPair<int, QPoint>> touchFileData() const;
    Q_INVOKABLE void clearTouchFileData();

    Q_INVOKABLE QHash<QUrl, QUrl> renameFileData() const;
    Q_INVOKABLE void removeRenameFileData(const QUrl &oldUrl);
    Q_INVOKABLE void clearRenameFileData();

    Q_INVOKABLE QSet<QUrl> pasteFileData() const;
    Q_INVOKABLE void removePasteFileData(const QUrl &oldUrl);
    Q_INVOKABLE void clearPasteFileData();
signals:
    void filePastedCallback();
    void fileTouchedCallback();
    void fileRenamedCallback();
public:
    void callBackFunction(const DFMBASE_NAMESPACE::AbstractJobHandler::CallbackArgus args);

protected:
    explicit FileOperatorProxy(QObject *parent = nullptr);

private:
    QSharedPointer<FileOperatorProxyPrivate> d;
};

#define FileOperatorProxyIns DDP_CANVAS_NAMESPACE::FileOperatorProxy::instance()

}

#endif   // FILEOPERATORPROXY_H
