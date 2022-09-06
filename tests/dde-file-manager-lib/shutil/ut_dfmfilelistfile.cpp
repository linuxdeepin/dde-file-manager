// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfmapplication.h"

#include "stub-ext/stubext.h"
#include "shutil/dfmfilelistfile.h"

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include <QDir>
#include <memory>

namespace  {
    class TestDFMFileListFile : public testing::Test {
    public:
        QString hide_file = QDir::homePath() + "/.local/share/applications/";
        void SetUp() override
        {
            mFilelistfile.reset(new DFMFileListFile(hide_file));
        }
        void TearDown() override
        {
        }

    public:
        std::shared_ptr<DFMFileListFile> mFilelistfile = nullptr;
    };
}

TEST_F(TestDFMFileListFile, filelist_contains_hiddenfile)
{
    EXPECT_TRUE(mFilelistfile->filePath().contains(".hidden"));
    EXPECT_EQ(hide_file,mFilelistfile->dirPath());
}

TEST_F(TestDFMFileListFile, filelist_can_add_newfile)
{
    QString add_file = QDir::homePath() + "/.local/share/applications/add_file.txt";

    mFilelistfile->insert(add_file);
    EXPECT_TRUE(mFilelistfile->contains(add_file));
}

TEST_F(TestDFMFileListFile, filelist_can_remove_onefile)
{
    QString add_file = QDir::homePath() + "/.local/share/applications/add_file.txt";

    mFilelistfile->insert(add_file);
    EXPECT_NO_FATAL_FAILURE(mFilelistfile->contains(add_file));

    mFilelistfile->remove(add_file);
    EXPECT_NO_FATAL_FAILURE(mFilelistfile->contains(add_file));
}

TEST_F(TestDFMFileListFile, can_relode_the_hidden_file)
{
    QString hidden_file = QDir::homePath() + "/.local/share/applications/no_exit_file.so";

    EXPECT_NO_FATAL_FAILURE(mFilelistfile->supportHideByFile(hidden_file));
    EXPECT_NO_FATAL_FAILURE(mFilelistfile->canHideByFile(hidden_file));
    EXPECT_NO_FATAL_FAILURE(mFilelistfile->reload());
}

TEST_F(TestDFMFileListFile, cant_relode_the_notexisted_file)
{
    QString not_file = QDir::homePath() + "/.local/share/applications/not_existed_file.so";

    DFMFileListFile nofile(not_file);
    EXPECT_FALSE(nofile.reload());
}

TEST_F(TestDFMFileListFile, load_file_get_attribute)
{
    QString not_file = QDir::homePath() + "/.local/share/applications/not_existed_file.so";

    DFMFileListFile nofile(not_file);

    stub_ext::StubExt stub;
    stub.set_lamda(ADDR(dde_file_manager::DFMApplication,genericAttribute),[](dde_file_manager::DFMApplication::GenericAttribute ga){
        Q_UNUSED(ga);
        return true;
    });

    EXPECT_FALSE(nofile.reload());
}

TEST_F(TestDFMFileListFile, load_file_with_mtp)
{
    QString not_file = QDir::homePath() + "/.local/share/applications/not_existed_file.so";

    DFMFileListFile nofile(not_file);

    stub_ext::StubExt stub;
    stub.set_lamda(ADDR(dde_file_manager::DFMApplication,genericAttribute),[](dde_file_manager::DFMApplication::GenericAttribute ga){
        Q_UNUSED(ga);
        return false;
    });

    Stub stub2;
    bool (*contains_ut)(const QString&, Qt::CaseSensitivity) = [](const QString&, Qt::CaseSensitivity){
        return true;
    };
    using ut_over = bool(QString::*)(const QString&, Qt::CaseSensitivity)const;
    stub2.set((ut_over)&QString::contains, contains_ut);

    EXPECT_FALSE(nofile.reload());
}
