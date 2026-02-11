// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "previewlibrary.h"
#include "config.h"

#include <QLibrary>
#include <QDir>

DFM_LOG_REGISTER_CATEGORY(DFMPREVIEW_LOG_CG)

PreviewLibrary::PreviewLibrary(QObject *parent)
    : QObject(parent)
{
}

bool PreviewLibrary::load()
{
    if (previewLib)
        return true;

    QString libPath;
    {
        //默认路径
#ifdef QT_DEBUG
        char path[PATH_MAX] = { 0 };
        const char *defaultPath = realpath("../libdfm-preview", path);
#else
        auto defaultPath = DFM_PLUGIN_PREVIEW_DIR;
#endif
        static_assert(std::is_same<decltype(defaultPath), const char *>::value, "DFM_PLUGIN_PREVIEW_DIR is not a string.");

        QDir dir(defaultPath);
        libPath = dir.absoluteFilePath("libdfm-preview.so");
    }

    previewLib = new QLibrary(libPath, this);
    if (!previewLib->load()) {
        fmCritical() << "Can not load libdfm-preview.so" << previewLib->errorString();
        delete previewLib;
        previewLib = nullptr;
        return false;
    }

    initFunc = (InitializePreview)previewLib->resolve("initFilePreview");
    if (!initFunc) {
        fmCritical() << "No such api initFilePreview in libdfm-preview.so";
        delete previewLib;
        previewLib = nullptr;
        return false;
    }

    showFunc = (ShowPreviewDialog)previewLib->resolve("showFilePreviewDialog");
    if (!showFunc) {
        fmCritical() << "No such api showFilePreviewDialog in libdfm-preview.so";
        delete previewLib;
        previewLib = nullptr;
        return false;
    }

    initFunc();
    return true;
}

void PreviewLibrary::unload()
{
    if (previewLib) {
        previewLib->unload();
        delete previewLib;
        previewLib = nullptr;
        showFunc = nullptr;
    }
}

void PreviewLibrary::showPreview(quint64 windowId, const QList<QUrl> &selecturls, const QList<QUrl> dirUrl)
{
    if (showFunc)
        showFunc(windowId, selecturls, dirUrl);
}
