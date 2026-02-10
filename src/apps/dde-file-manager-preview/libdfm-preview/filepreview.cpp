// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
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
    qCInfo(logLibFilePreview) << "FilePreview: initializing file preview service";
    filePreviewIns = new FilePreview;
    filePreviewIns->initialize();
    bool started = filePreviewIns->start();
    qCInfo(logLibFilePreview) << "FilePreview: initialization completed, service started:" << started;

    return 0;
}

int showFilePreviewDialog(quint64 windowId, const QList<QUrl> &selecturls, const QList<QUrl> dirUrl)
{
    qCInfo(logLibFilePreview) << "FilePreview: received preview request for window ID:" << windowId 
                              << "with" << selecturls.size() << "selected files and" << dirUrl.size() << "directory files";
    
    if (filePreviewIns) {
        filePreviewIns->showFilePreview(windowId, selecturls, dirUrl);
    } else {
        qCCritical(logLibFilePreview) << "FilePreview: service instance is null, cannot show preview dialog";
    }
    return 0;
}

#ifdef __cplusplus
}
#endif

DFM_LOG_REGISTER_CATEGORY(DPFILEPREVIEW_NAMESPACE)
void FilePreview::initialize()
{
    qCInfo(logLibFilePreview) << "FilePreview: starting initialization process";
    
    // Under the wayland protocol, set the rendering mode to OpenGLES
    if (WindowUtils::isWayLand()) {
        qCInfo(logLibFilePreview) << "FilePreview: detected Wayland environment";
#ifndef __x86_64__
        qCInfo(logLibFilePreview) << "FilePreview: setting OpenGLES rendering format for non-x86_64 architecture";
        QSurfaceFormat format;
        format.setRenderableType(QSurfaceFormat::OpenGLES);
        format.setDefaultFormat(format);
#else
        qCDebug(logLibFilePreview) << "FilePreview: x86_64 architecture detected, using default rendering format";
#endif
    } else {
        qCDebug(logLibFilePreview) << "FilePreview: detected X11 environment";
    }

    qCDebug(logLibFilePreview) << "FilePreview: registering URL schemes and factories";
    UrlRoute::regScheme(Global::Scheme::kFile, "/");
    InfoFactory::regInfoTransFunc<FileInfo>(Global::Scheme::kFile, DesktopFileInfo::convert);
    UrlRoute::regScheme(Global::Scheme::kAsyncFile, "/", QIcon(), false, QObject::tr("System Disk"));
    UrlRoute::regScheme(Global::Scheme::kAsyncFile, "/");
    InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
    InfoFactory::regClass<AsyncFileInfo>(Global::Scheme::kAsyncFile);
    DirIteratorFactory::regClass<LocalDirIterator>(Global::Scheme::kFile);
    WatcherFactory::regClass<LocalFileWatcher>(Global::Scheme::kFile);
    
    qCInfo(logLibFilePreview) << "FilePreview: initialization process completed";
}

bool FilePreview::start()
{
    qCInfo(logLibFilePreview) << "FilePreview: starting service";
    
    QString err;
    auto ret = DConfigManager::instance()->addConfig(ConfigInfos::kConfName, &err);
    if (!ret) {
        qCWarning(logLibFilePreview) << "FilePreview: failed to create dconfig:" << err;
        return false;
    } else {
        qCInfo(logLibFilePreview) << "FilePreview: dconfig created successfully";
    }

    bool previewEnabled = isPreviewEnabled();
    qCInfo(logLibFilePreview) << "FilePreview: preview feature enabled:" << previewEnabled;
    
    return true;
}

void FilePreview::showFilePreview(quint64 windowId, const QList<QUrl> &selecturls, const QList<QUrl> dirUrl)
{
    qCDebug(logLibFilePreview) << "FilePreview: checking if preview is enabled before showing dialog";
    
    if (isPreviewEnabled()) {
        qCInfo(logLibFilePreview) << "FilePreview: preview enabled, delegating to PreviewDialogManager";
        PreviewDialogManager::instance()->showPreviewDialog(windowId, selecturls, dirUrl);
    } else {
        qCWarning(logLibFilePreview) << "FilePreview: preview is disabled, ignoring preview request";
    }
}

bool FilePreview::isPreviewEnabled()
{
    const auto &&ret = DConfigManager::instance()->value(ConfigInfos::kConfName, "previewEnable");
    return ret.isValid() ? ret.toBool() : true;
}
