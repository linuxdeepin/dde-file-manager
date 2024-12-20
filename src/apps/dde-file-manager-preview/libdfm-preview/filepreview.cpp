// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filepreview.h"
#include "utils/previewdialogmanager.h"

#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/utils/windowutils.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/file/local/asyncfileinfo.h>
#include <dfm-base/file/local/localdiriterator.h>
#include <dfm-base/file/local/localfilewatcher.h>
#include <dfm-base/file/local/desktopfileinfo.h>

#include <QSurfaceFormat>

Q_LOGGING_CATEGORY(logLibFilePreview, "org.deepin.dde.FilePreview")
DFMBASE_USE_NAMESPACE
DPFILEPREVIEW_USE_NAMESPACE

static FilePreview *filePreviewIns = nullptr;

#ifdef __cplusplus
extern "C" {
#endif

int initFilePreview()
{
    filePreviewIns = new FilePreview;
    filePreviewIns->initialize();
    filePreviewIns->start();

    return 0;
}

int showFilePreviewDialog(quint64 windowId, const QList<QUrl> &selecturls, const QList<QUrl> dirUrl)
{
    if (filePreviewIns)
        filePreviewIns->showFilePreview(windowId, selecturls, dirUrl);
    return 0;
}

#ifdef __cplusplus
}
#endif

DFM_LOG_REISGER_CATEGORY(DPFILEPREVIEW_NAMESPACE)
void FilePreview::initialize()
{
    // Under the wayland protocol, set the rendering mode to OpenGLES
    if (WindowUtils::isWayLand()) {
#ifndef __x86_64__
        QSurfaceFormat format;
        format.setRenderableType(QSurfaceFormat::OpenGLES);
        format.setDefaultFormat(format);
#endif
    }

    UrlRoute::regScheme(Global::Scheme::kFile, "/");
    InfoFactory::regInfoTransFunc<FileInfo>(Global::Scheme::kFile, DesktopFileInfo::convert);
    UrlRoute::regScheme(Global::Scheme::kAsyncFile, "/", QIcon(), false, QObject::tr("System Disk"));
    UrlRoute::regScheme(Global::Scheme::kAsyncFile, "/");
    InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
    InfoFactory::regClass<AsyncFileInfo>(Global::Scheme::kAsyncFile);
    DirIteratorFactory::regClass<LocalDirIterator>(Global::Scheme::kFile);
    WatcherFactory::regClass<LocalFileWatcher>(Global::Scheme::kFile);
}

bool FilePreview::start()
{
    QString err;
    auto ret = DConfigManager::instance()->addConfig(ConfigInfos::kConfName, &err);
    if (!ret)
        // fmWarning() << "File Preview: create dconfig failed: " << err;
        qCWarning(logLibFilePreview()) << "File Preview: create dconfig failed: " << err;

    return true;
}

void FilePreview::showFilePreview(quint64 windowId, const QList<QUrl> &selecturls, const QList<QUrl> dirUrl)
{
    if (isPreviewEnabled())
        PreviewDialogManager::instance()->showPreviewDialog(windowId, selecturls, dirUrl);
}

bool FilePreview::isPreviewEnabled()
{
    const auto &&ret = DConfigManager::instance()->value(ConfigInfos::kConfName, "previewEnable");
    return ret.isValid() ? ret.toBool() : true;
}
