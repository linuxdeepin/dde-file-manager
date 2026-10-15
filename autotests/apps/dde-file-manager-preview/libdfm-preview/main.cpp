// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Custom main for ut-libdfm-preview:
// 1. points PreviewPluginLoaderPrivate::pluginPaths at the built fake plugin
//    BEFORE any PreviewPluginLoader exists;
// 2. registers the "file" scheme so InfoFactory works without the full plugin
//    framework being loaded.

#include <gtest/gtest.h>

#include <QApplication>

#include "dfm_test_main.h"

#include "pluginInterface/private/pluginloader_p.h"

#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/file/local/asyncfileinfo.h>
#include <dfm-base/file/local/desktopfileinfo.h>
#include <dfm-base/file/local/localdiriterator.h>
#include <dfm-base/file/local/localfilewatcher.h>

int main(int argc, char **argv)
{
    // the fake preview plugin is built next to this test binary
    dfmplugin_filepreview::PreviewPluginLoaderPrivate::pluginPaths =
            QStringList { QString(UT_FAKE_PLUGIN_DIR) };

    QApplication app(argc, argv);

    DFMBASE_USE_NAMESPACE
    UrlRoute::regScheme(Global::Scheme::kFile, "/");
    UrlRoute::regScheme(Global::Scheme::kAsyncFile, "/", QIcon(), false, QObject::tr("System Disk"));
    InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
    InfoFactory::regClass<AsyncFileInfo>(Global::Scheme::kAsyncFile);
    DirIteratorFactory::regClass<LocalDirIterator>(Global::Scheme::kFile);
    WatcherFactory::regClass<LocalFileWatcher>(Global::Scheme::kFile);

    ::testing::InitGoogleTest(&argc, argv);
    const int result = RUN_ALL_TESTS();
    DFM_SETUP_ASAN_REPORT(libdfm_preview);
    return result;
}
