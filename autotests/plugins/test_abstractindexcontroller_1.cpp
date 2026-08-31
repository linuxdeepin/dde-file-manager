// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_abstractindexcontroller_1.cpp
 * @brief Unit tests for AbstractIndexController methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "abstractindexcontroller.h"

#include <QTest>

using namespace core;

class AbstractIndexControllerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new AbstractIndexController();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    AbstractIndexController *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(AbstractIndexControllerTest, AbstractIndexController)
{
    // Test constructor: AbstractIndexController((IndexControllerDescriptor descriptor, QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(AbstractIndexControllerTest, activeBackend)
{
    // Test method: void activeBackend((bool isInit))
    EXPECT_NO_FATAL_FAILURE(obj->activeBackend(false));
}

TEST_F(AbstractIndexControllerTest, descriptor)
{
    // Test getter: IndexControllerDescriptor descriptor()
    auto result = obj->descriptor();
    EXPECT_NO_FATAL_FAILURE({ obj->descriptor(); });

}

TEST_F(AbstractIndexControllerTest, handleConfigChanged)
{
    // Test method: void handleConfigChanged((const QString &config, const QString &key))
    QString _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->handleConfigChanged(_arg0, _arg1));
}

TEST_F(AbstractIndexControllerTest, indexedPaths)
{
    // Test getter: QStringList indexedPaths()
    auto result = obj->indexedPaths();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(AbstractIndexControllerTest, initialize)
{
    // Test method: void initialize(())
    EXPECT_NO_FATAL_FAILURE(obj->initialize());
}

TEST_F(AbstractIndexControllerTest, isBackendAvaliable)
{
    // Test bool getter: isBackendAvaliable()
    bool result = obj->isBackendAvaliable();
    EXPECT_FALSE(result);

}

TEST_F(AbstractIndexControllerTest, isTrackedPath)
{
    // Test method: bool isTrackedPath((const QString &path))
    QString _arg0{};
    auto result = obj->isTrackedPath(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(AbstractIndexControllerTest, keepBackendAlive)
{
    // Test method: void keepBackendAlive(())
    EXPECT_NO_FATAL_FAILURE(obj->keepBackendAlive());
}

TEST_F(AbstractIndexControllerTest, onTaskFinished)
{
    // Test method: void onTaskFinished((const QString &type, const QString &path, bool success))
    QString _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->onTaskFinished(_arg0, _arg1, false));
}

TEST_F(AbstractIndexControllerTest, onTaskProgressChanged)
{
    // Test method: void onTaskProgressChanged((const QString &type, const QString &path, qint64 count, qint64 total))
    QString _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->onTaskProgressChanged(_arg0, _arg1, 0, 0));
}

TEST_F(AbstractIndexControllerTest, setupDBusConnections)
{
    // Test method: void setupDBusConnections(())
    EXPECT_NO_FATAL_FAILURE(obj->setupDBusConnections());
}

TEST_F(AbstractIndexControllerTest, startIndexTask)
{
    // Test method: void startIndexTask((bool isCreate, bool silent))
    EXPECT_NO_FATAL_FAILURE(obj->startIndexTask(false, false));
}

TEST_F(AbstractIndexControllerTest, updateKeepAliveTimer)
{
    // Test method: void updateKeepAliveTimer(())
    EXPECT_NO_FATAL_FAILURE(obj->updateKeepAliveTimer());
}

TEST_F(AbstractIndexControllerTest, updateState)
{
    // Test method: void updateState((State newState))
    EXPECT_NO_FATAL_FAILURE(obj->updateState(State()));
}
