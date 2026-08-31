// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_docutfilesworker.cpp
 * @brief Unit tests for DoCutFilesWorker methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "fileoperations/cutfiles/docutfilesworker.h"

#include <QTest>

using namespace dfmplugin_fileoperations;

class DoCutFilesWorkerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DoCutFilesWorker();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DoCutFilesWorker *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DoCutFilesWorkerTest, doMergDir)
{
    // Test method: bool doMergDir((const DFileInfoPointer &fromInfo, const DFileInfoPointer &toInfo, bool *skip))
    DFileInfoPointer _arg0{};
    DFileInfoPointer _arg1{};
    auto result = obj->doMergDir(_arg0, _arg1, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(DoCutFilesWorkerTest, trySameDeviceRename)
{
    // Test method: DFileInfoPointer trySameDeviceRename((const DFileInfoPointer &sourceInfo,
                                                       const DFileInfoPointer &targetPathInfo,
                                                       const QString fileName, bool *ok, bool *skip))
    DFileInfoPointer _arg0{};
    DFileInfoPointer _arg1{};
    auto result = obj->trySameDeviceRename(_arg0, _arg1, QString(), nullptr, nullptr);
    EXPECT_NE(result.get(), nullptr);

}
