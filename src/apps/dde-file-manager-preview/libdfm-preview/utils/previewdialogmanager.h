// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PREVIEWDIALOGMANAGER_H
#define PREVIEWDIALOGMANAGER_H

#include "dfmplugin_filepreview_global.h"
#include "views/filepreviewdialog.h"

#include <QObject>

namespace dfmplugin_filepreview {
class PreviewDialogManager : public QObject
{
    Q_OBJECT
public:
    static PreviewDialogManager *instance();

public Q_SLOTS:
    void onPreviewDialogClose();

private:
    explicit PreviewDialogManager(QObject *parent = nullptr);

public slots:
    void showPreviewDialog(const quint64 winId, const QList<QUrl> &selecturls, const QList<QUrl> &dirUrl);

private:
    FilePreviewDialog *filePreviewDialog { nullptr };
    QTimer *exitTimer { nullptr };
};
}
#endif   // PREVIEWDIALOGMANAGER_H
