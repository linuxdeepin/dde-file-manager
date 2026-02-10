// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PREVIEWLIBRARY_H
#define PREVIEWLIBRARY_H

#include <dfm-base/dfm_log_defines.h>

#include <QObject>
#include <QUrl>

#define DFMPREVIEW_LOG_CG dde_file_manager_preview
DFM_LOG_USE_CATEGORY(DFMPREVIEW_LOG_CG)

typedef int (*ShowPreviewDialog)(quint64 windowId, const QList<QUrl> &selecturls, const QList<QUrl> dirUrl);
typedef int (*InitializePreview)();
class QLibrary;

class PreviewLibrary: public QObject
{
    Q_OBJECT
public:
    explicit PreviewLibrary(QObject *parent = nullptr);

    bool load();
    void unload();
    void showPreview(quint64 windowId, const QList<QUrl> &selecturls, const QList<QUrl> dirUrl);

protected:
    ShowPreviewDialog showFunc = nullptr;
    InitializePreview initFunc = nullptr;
    QLibrary *previewLib = nullptr;
};

#endif // PREVIEWLIBRARY_H
