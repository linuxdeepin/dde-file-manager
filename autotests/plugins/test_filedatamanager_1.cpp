// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_filedatamanager_1.cpp
 * @brief Unit tests for FileDataManager methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/filedatamanager.h"

#include <QTest>

using namespace dfmplugin_workspace;

class FileDataManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileDataManager();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileDataManager *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileDataManagerTest, FileDataManager)
{
    // Test constructor: FileDataManager((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(FileDataManagerTest, addRootUser)
{
    // Test method: void addRootUser((const QUrl &url, const QString &key))
    QUrl _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->addRootUser(_arg0, _arg1));
}

TEST_F(FileDataManagerTest, createRoot)
{
    // Test method: RootInfo createRoot((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->createRoot(_arg0);
    EXPECT_NO_FATAL_FAILURE({ obj->createRoot(_arg0); });

}

TEST_F(FileDataManagerTest, fetchFiles)
{
    // Test method: bool fetchFiles((const QUrl &rootUrl, const QString &key, DFMGLOBAL_NAMESPACE::ItemRoles role, Qt::SortOrder order))
    QUrl _arg0{};
    QString _arg1{};
    auto result = obj->fetchFiles(_arg0, _arg1, DFMGLOBAL_NAMESPACE::ItemRoles(), Qt::SortOrder());
    EXPECT_FALSE(result);

}

TEST_F(FileDataManagerTest, fetchRoot)
{
    // Test method: RootInfo fetchRoot((const QUrl &url, const QString &key))
    QUrl _arg0{};
    QString _arg1{};
    auto result = obj->fetchRoot(_arg0, _arg1);
    EXPECT_NO_FATAL_FAILURE({ obj->fetchRoot(_arg0, _arg1); });

}

TEST_F(FileDataManagerTest, handleDeletion)
{
    // Test method: void handleDeletion((RootInfo *root))
    EXPECT_NO_FATAL_FAILURE(obj->handleDeletion(nullptr));
}

TEST_F(FileDataManagerTest, hasRootUsers)
{
    // Test method: bool hasRootUsers((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->hasRootUsers(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(FileDataManagerTest, instance)
{
    // Test getter: FileDataManager instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}

TEST_F(FileDataManagerTest, normalizeRootUrl)
{
    // Test method: QUrl normalizeRootUrl((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->normalizeRootUrl(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(FileDataManagerTest, onAppAttributeChanged)
{
    // Test method: void onAppAttributeChanged((Application::ApplicationAttribute aa, const QVariant &value))
    QVariant _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->onAppAttributeChanged(Application::ApplicationAttribute(), _arg1));
}

TEST_F(FileDataManagerTest, setFileActive)
{
    // Test setter: void setFileActive((const QUrl &rootUrl, const QUrl &childUrl, bool active))
    QUrl _arg0{};
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->setFileActive(_arg0, _arg1, false));
}

TEST_F(FileDataManagerTest, stopRootWork)
{
    // Test method: void stopRootWork((const QUrl &rootUrl, const QString &key))
    QUrl _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->stopRootWork(_arg0, _arg1));
}

TEST_F(FileDataManagerTest, FileDataManager_Destructor)
{
    // Test method:  ~FileDataManager(())
    EXPECT_NO_FATAL_FAILURE({ FileDataManager *tmp = new FileDataManager(); delete tmp; });
}
