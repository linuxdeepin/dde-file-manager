// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "stubext.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/dfm_global_defines.h>

#include "fileoperations/trashfiles/copyfromtrashfiles.h"
#include "fileoperations/trashfiles/docopyfromtrashfilesworker.h"

DFMBASE_USE_NAMESPACE
DPFILEOPERATIONS_USE_NAMESPACE

class TestCopyFromTrashFiles : public testing::Test
{
public:
    void SetUp() override
    {
        // Register FileInfo classes
        UrlRoute::regScheme(Global::Scheme::kFile, "/");
        InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);

        // Create copyFromTrashFiles instance
        copyFromTrashFiles = new CopyFromTrashTrashFiles();
        ASSERT_TRUE(copyFromTrashFiles);
    }

    void TearDown() override
    {
        stub.clear();
        if (copyFromTrashFiles) {
            delete copyFromTrashFiles;
            copyFromTrashFiles = nullptr;
        }
    }

protected:
    stub_ext::StubExt stub;
    CopyFromTrashTrashFiles *copyFromTrashFiles;
};

// ========== Constructor/Destructor Tests ==========

TEST_F(TestCopyFromTrashFiles, Constructor_CreatesWorker)
{
    EXPECT_TRUE(copyFromTrashFiles);
    // Worker should be created internally
}

TEST_F(TestCopyFromTrashFiles, Destructor_Cleanup)
{
    delete copyFromTrashFiles;
    copyFromTrashFiles = nullptr;
    // Should cleanup without crashes
}
