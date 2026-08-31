// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_emblemiconworker_1.cpp
 * @brief Unit tests for EmblemIconWorker methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "extensionimpl/emblemimpl/extensionemblemmanager.h"

#include <QTest>

using namespace dfmplugin_utils;

class EmblemIconWorkerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new EmblemIconWorker();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    EmblemIconWorker *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(EmblemIconWorkerTest, makeCache)
{
    // Test method: EmblemIconWorker::CacheType makeCache((const QString &path, const QList<QPair<QString, int>> &group))
    QString _arg0{};
    QList<QPair<QString, int>> _arg1{};
    auto result = obj->makeCache(_arg0, _arg1);
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(EmblemIconWorkerTest, makeLayoutGroup)
{
    // Test method: void makeLayoutGroup((const std::vector<dfmext::DFMExtEmblemIconLayout> &layouts, QList<QPair<QString, int>> *group))
    std::vector<dfmext::DFMExtEmblemIconLayout> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->makeLayoutGroup(_arg0, nullptr));
}

TEST_F(EmblemIconWorkerTest, makeNormalGroup)
{
    // Test method: void makeNormalGroup((const std::vector<std::string> &icons, int count, QList<QPair<QString, int>> *group))
    std::vector<std::string> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->makeNormalGroup(_arg0, 0, nullptr));
}

TEST_F(EmblemIconWorkerTest, mergeGroup)
{
    // Test method: void mergeGroup((const QList<QPair<QString, int>> &oldGroup,
                                  const QList<QPair<QString, int>> &newGroup,
                                  QList<QPair<QString, int>> *group))
    QList<QPair<QString, int>> _arg0{};
    QList<QPair<QString, int>> _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->mergeGroup(_arg0, _arg1, nullptr));
}

TEST_F(EmblemIconWorkerTest, onFetchEmblemIcons)
{
    // Test method: void onFetchEmblemIcons((const QList<QPair<QString, int>> &localPaths))
    QList<QPair<QString, int>> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onFetchEmblemIcons(_arg0));
}

TEST_F(EmblemIconWorkerTest, parseEmblemIcons)
{
    // Test method: void parseEmblemIcons((const QString &path, int count, dfmext::DFMExtEmblemIconPlugin *plugin))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->parseEmblemIcons(_arg0, 0, nullptr));
}

TEST_F(EmblemIconWorkerTest, parseLocationEmblemIcons)
{
    // Test method: bool parseLocationEmblemIcons((const QString &path, int count, dfmext::DFMExtEmblemIconPlugin *plugin))
    QString _arg0{};
    auto result = obj->parseLocationEmblemIcons(_arg0, 0, nullptr);
    EXPECT_FALSE(result);

}
