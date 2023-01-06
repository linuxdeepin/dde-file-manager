// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include "stub.h"
#include "stubext.h"

#include "controllers/dfmappentrycontroller.h"
#include "models/dfmappentryfileinfo.h"
#include "dfmstandardpaths.h"
#include "dfmevent.h"
#include "dabstractfilewatcher.h"

#include <QDir>

namespace {
class TestDFMAppEntryController : public testing::Test
{
public:
    void SetUp() override
    {
        std::cout << "start TestDFMAppEntryController" << std::endl;
    }

    void TearDown() override
    {
        std::cout << "end TestDFMAppEntryController" << std::endl;
    }

public:
    DFMAppEntryController controller;
};
} // namespace

TEST_F(TestDFMAppEntryController, tst_getChildren) {
    stub_ext::StubExt stub;
    stub.set_lamda((bool(QDir::*)()const)&QDir::exists, [](void *){ return true; });
    stub.set_lamda((QStringList(QDir::*)(const QStringList &, QDir::Filters, QDir::SortFlags) const)
                   &QDir::entryList, [](void *, const QStringList &, QDir::Filters, QDir::SortFlags){
        QStringList ret;
        ret << "/usr/share/applications/deepin-draw.desktop";
        ret << "/usr/share/applications/deepin-editor.desktop";
        ret << "/this/is/not/exist/item";
        return ret;
    });
    stub.set_lamda(&DFMStandardPaths::location, []{
        return "/usr/share/applications/";
    });
    stub.set_lamda((bool(*)(DFMAppEntryFileInfo *))
                   (&DFMAppEntryFileInfo::exists), [](void *){ return true; });

    EXPECT_TRUE(controller.getChildren({}).count() > 0);
}

TEST_F(TestDFMAppEntryController, tst_createFileInfo) {
    auto event = dMakeEventPointer<DFMCreateFileInfoEvent>(nullptr, DUrl("/usr/share/applications/deepin-draw.desktop"));
    EXPECT_TRUE(controller.createFileInfo(event));
}

TEST_F(TestDFMAppEntryController, tst_createFileWatcher) {
    auto watcher = controller.createFileWatcher({});
    EXPECT_TRUE(watcher);
    delete watcher;
}

TEST_F(TestDFMAppEntryController, tst_openFile) {

}

TEST_F(TestDFMAppEntryController, tst_localToAppEntry) {
    EXPECT_NO_FATAL_FAILURE(controller.localToAppEntry(DUrl("/test/dir")));

    stub_ext::StubExt stub;
    stub.set_lamda(&DFMStandardPaths::location, []{
        return "/usr/share/applications/";
    });
    EXPECT_NO_FATAL_FAILURE(controller.localToAppEntry(DUrl("/usr/share/applications/notexistfile")));
    EXPECT_NO_FATAL_FAILURE(controller.localToAppEntry(DUrl("/usr/share/applications/notexistfile.desktop")));

    stub.set_lamda(&QFileInfo::isDir, []{ return false; });
    EXPECT_NO_FATAL_FAILURE(controller.localToAppEntry(DUrl("/usr/share/applications/notexistfile.desktop")));
}
