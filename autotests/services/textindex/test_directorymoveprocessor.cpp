// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_directorymoveprocessor.cpp
 * @brief Unit tests for DirectoryMoveProcessor methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/textindex/task/moveprocessor.h"

#include <QTest>

using namespace src;

class DirectoryMoveProcessorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DirectoryMoveProcessor();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DirectoryMoveProcessor *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DirectoryMoveProcessorTest, hasChanges)
{
    // Test bool getter: hasChanges()
    bool result = obj->hasChanges();
    EXPECT_FALSE(result);

}

TEST_F(DirectoryMoveProcessorTest, updateSingleDocumentPath)
{
    // Test method: bool updateSingleDocumentPath((const DocumentPtr &doc,
                                                      const QString &normalizedFromPath,
                                                      const QString &toPath))
    DocumentPtr _arg0{};
    QString _arg1{};
    QString _arg2{};
    auto result = obj->updateSingleDocumentPath(_arg0, _arg1, _arg2);
    EXPECT_FALSE(result);

}
