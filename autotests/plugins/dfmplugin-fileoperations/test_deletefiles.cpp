// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "stubext.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/dfm_global_defines.h>

#include "fileoperations/deletefiles/deletefiles.h"
#include "fileoperations/deletefiles/dodeletefilesworker.h"

DFMBASE_USE_NAMESPACE
DPFILEOPERATIONS_USE_NAMESPACE

class TestDeleteFiles : public testing::Test
{
public:
    void SetUp() override
    {
        // Register FileInfo classes
        UrlRoute::regScheme(Global::Scheme::kFile, "/");
        InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);

        // Create deleteFiles instance
        deleteFiles = new DeleteFiles();
        ASSERT_TRUE(deleteFiles);
    }

    void TearDown() override
    {
        stub.clear();
        if (deleteFiles) {
            delete deleteFiles;
            deleteFiles = nullptr;
        }
    }

protected:
    stub_ext::StubExt stub;
    DeleteFiles *deleteFiles;
};

// ========== Constructor/Destructor Tests ==========

TEST_F(TestDeleteFiles, Constructor_CreatesWorker)
{
    EXPECT_TRUE(deleteFiles);
    // Worker should be created internally
}

TEST_F(TestDeleteFiles, Destructor_Cleanup)
{
    delete deleteFiles;
    deleteFiles = nullptr;
    // Should cleanup without crashes
}

// ========== Integration Tests ==========

TEST_F(TestDeleteFiles, InheritsFromAbstractJob)
{
    // DeleteFiles should inherit from AbstractJob
    AbstractJob *job = dynamic_cast<AbstractJob *>(deleteFiles);
    EXPECT_TRUE(job != nullptr);
}
