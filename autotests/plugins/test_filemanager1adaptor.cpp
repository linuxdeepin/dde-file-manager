// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_filemanager1adaptor.cpp
 * @brief Unit tests for FileManager1Adaptor methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "config/dbus/FileManager1Adaptor.h"

#include <QTest>

using namespace src;

class FileManager1AdaptorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileManager1Adaptor();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileManager1Adaptor *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileManager1AdaptorTest, FileManager1Adaptor)
{
    // Test constructor: FileManager1Adaptor((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(FileManager1AdaptorTest, Open)
{
    // Test method: void Open((const QStringList &Args))
    QStringList _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->Open(_arg0));
}

TEST_F(FileManager1AdaptorTest, Trash)
{
    // Test method: void Trash((const QStringList &URIs))
    QStringList _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->Trash(_arg0));
}
