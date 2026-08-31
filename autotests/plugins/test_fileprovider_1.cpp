// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fileprovider_1.cpp
 * @brief Unit tests for FileProvider methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/textindex/task/fileprovider.h"

#include <QTest>

using namespace src;

class FileProviderTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileProvider();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileProvider *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileProviderTest, FileProvider)
{
    // Test constructor: FileProvider((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(FileProviderTest, installFileFilter)
{
    // Test method: void installFileFilter((QSharedPointer<FileFilter> filter))
    EXPECT_NO_FATAL_FAILURE(obj->installFileFilter(QSharedPointer<FileFilter>()));
}

TEST_F(FileProviderTest, isUpdating)
{
    // Test bool getter: isUpdating()
    bool result = obj->isUpdating();
    EXPECT_FALSE(result);

}

TEST_F(FileProviderTest, onFileInfoUpdated)
{
    // Test method: void onFileInfoUpdated((const QUrl &url, const QString &infoPtr, const bool isLinkOrg))
    QUrl _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->onFileInfoUpdated(_arg0, _arg1, false));
}

TEST_F(FileProviderTest, preupdateData)
{
    // Test method: void preupdateData((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->preupdateData(_arg0));
}

TEST_F(FileProviderTest, refresh)
{
    // Test method: void refresh((QDir::Filters filters))
    EXPECT_NO_FATAL_FAILURE(obj->refresh(QDir::Filters()));
}

TEST_F(FileProviderTest, setRoot)
{
    // Test method: bool setRoot((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->setRoot(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(FileProviderTest, traversalFinished)
{
    // Test method: void traversalFinished(())
    EXPECT_NO_FATAL_FAILURE(obj->traversalFinished());
}

TEST_F(FileProviderTest, update)
{
    // Test method: void update((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->update(_arg0));
}

TEST_F(FileProviderTest, FileProvider_Destructor)
{
    // Test method:  ~FileProvider(())
    EXPECT_NO_FATAL_FAILURE({ FileProvider *tmp = new FileProvider(); delete tmp; });
}
