// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include "stub.h"
#include "stubext.h"

#include "models/dfmappentryfileinfo.h"
#include "models/dfmappentryfileinfo_p.h"
#include "dfmstandardpaths.h"

namespace {
class TestDFMAppEntryFileInfo : public testing::Test
{
public:
    void SetUp() override
    {
        std::cout << "start TestDFMAppEntryFileInfo" << std::endl;
    }

    void TearDown() override
    {
        std::cout << "end TestDFMAppEntryFileInfo" << std::endl;
    }

public:
};
} // namespace

DFMAppEntryFileInfo *makeInfo() {
    stub_ext::StubExt stub;
    stub.set_lamda(&DFMStandardPaths::location, []{
        return "/usr/share/applications/";
    });

    auto info = new DFMAppEntryFileInfo(DUrl("appentry:///deepin-draw"));
    return info;
}

TEST_F(TestDFMAppEntryFileInfo, tst_suffix) {
    auto info = makeInfo();
    EXPECT_TRUE(info->suffix() == SUFFIX_APP_ENTRY);
    delete info;
}

TEST_F(TestDFMAppEntryFileInfo, tst_fileDisplayName) {
    auto info = makeInfo();
    EXPECT_FALSE(info->fileDisplayName().isEmpty());
    delete info;
}

TEST_F(TestDFMAppEntryFileInfo, tst_iconName) {
    auto info = makeInfo();
    EXPECT_FALSE(info->iconName().isEmpty());
    delete info;
}

TEST_F(TestDFMAppEntryFileInfo, tst_exists) {
    auto info = makeInfo();
    EXPECT_TRUE(info->exists());
    delete info;
}

TEST_F(TestDFMAppEntryFileInfo, tst_fileType) {
    auto info = makeInfo();
    EXPECT_TRUE(info->fileType() == static_cast<DFMRootFileInfo::FileType>(DFMRootFileInfo::ItemType::AppEntry));
    delete info;
}

TEST_F(TestDFMAppEntryFileInfo, tst_menuActionList) {
    auto info = makeInfo();
    EXPECT_TRUE(info->menuActionList(DFMAppEntryFileInfo::MenuType(0)).count() == 1);
    delete info;
}

TEST_F(TestDFMAppEntryFileInfo, tst_cmd) {
    auto info = makeInfo();
    EXPECT_FALSE(info->cmd().isEmpty());
    delete info;
}

TEST_F(TestDFMAppEntryFileInfo, tst_executableBin) {
    auto info = makeInfo();
    EXPECT_FALSE(info->executableBin().isEmpty());
    delete info;
}
