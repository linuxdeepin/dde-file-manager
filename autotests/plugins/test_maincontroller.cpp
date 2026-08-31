// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_maincontroller.cpp
 * @brief Unit tests for MainController methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "searchmanager/maincontroller/maincontroller.h"

#include <QTest>

using namespace dfmplugin_search;

class MainControllerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new MainController();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    MainController *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(MainControllerTest, doSearchTask)
{
    // Test method: bool doSearchTask((QString taskId, const QUrl &url, const QString &keyword))
    QUrl _arg1{};
    QString _arg2{};
    auto result = obj->doSearchTask(QString(), _arg1, _arg2);
    EXPECT_FALSE(result);

}

TEST_F(MainControllerTest, getResultUrls)
{
    // Test method: QList<QUrl> getResultUrls((QString taskId))
    auto result = obj->getResultUrls(QString());
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(MainControllerTest, getResults)
{
    // Test method: DFMSearchResultMap getResults((QString taskId))
    auto result = obj->getResults(QString());
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(MainControllerTest, onFinished)
{
    // Test method: void onFinished((QString taskId))
    EXPECT_NO_FATAL_FAILURE(obj->onFinished(QString()));
}

TEST_F(MainControllerTest, stop)
{
    // Test method: void stop((QString taskId))
    EXPECT_NO_FATAL_FAILURE(obj->stop(QString()));
}

TEST_F(MainControllerTest, MainController_Destructor)
{
    // Test method:  ~MainController(())
    EXPECT_NO_FATAL_FAILURE({ MainController *tmp = new MainController(); delete tmp; });
}
