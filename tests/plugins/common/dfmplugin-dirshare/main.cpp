// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <dfm-base/base/application/application.h>

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/file/local/localdiriterator.h>
#include <dfm-base/file/local/localfilewatcher.h>

#include <gtest/gtest.h>
#include <sanitizer/asan_interface.h>
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    auto ins = new dfmbase::Application();
    DFMBASE_USE_NAMESPACE
    UrlRoute::regScheme(Global::Scheme::kFile, "/", QIcon(), false, QObject::tr("System Disk"));
    InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
    WatcherFactory::regClass<LocalFileWatcher>(Global::Scheme::kFile);

    ::testing::InitGoogleTest(&argc, argv);

    int ret = RUN_ALL_TESTS();

    delete ins;

#ifdef ENABLE_TSAN_TOOL
    __sanitizer_set_report_path("../../../asan_dfmplugin-dirshare.log");
#endif

    return ret;
}
