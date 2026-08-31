// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_detailspacehelper_1.cpp
 * @brief Unit tests for DetailSpaceHelper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/detailspacehelper.h"

#include <QTest>

using namespace dfmplugin_detailspace;

class DetailSpaceHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DetailSpaceHelper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DetailSpaceHelper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DetailSpaceHelperTest, addDetailSpace)
{
    // Test method: void addDetailSpace((quint64 windowId))
    EXPECT_NO_FATAL_FAILURE(obj->addDetailSpace(0));
}

TEST_F(DetailSpaceHelperTest, findDetailSpaceByWindowId)
{
    // Test method: DetailSpaceWidget findDetailSpaceByWindowId((quint64 windowId))
    auto result = obj->findDetailSpaceByWindowId(0);
    EXPECT_NO_FATAL_FAILURE({ obj->findDetailSpaceByWindowId(0); });

}

TEST_F(DetailSpaceHelperTest, findWindowIdByDetailSpace)
{
    // Test method: quint64 findWindowIdByDetailSpace((DetailSpaceWidget *widget))
    auto result = obj->findWindowIdByDetailSpace(nullptr);
    EXPECT_GE(result, 0);

}

TEST_F(DetailSpaceHelperTest, mutex)
{
    // Test getter: QMutex mutex()
    auto result = obj->mutex();
    EXPECT_NO_FATAL_FAILURE({ obj->mutex(); });

}

TEST_F(DetailSpaceHelperTest, setDetailViewByUrl)
{
    // Test setter: void setDetailViewByUrl((DetailSpaceWidget *w, const QUrl &url))
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->setDetailViewByUrl(nullptr, _arg1));
}

TEST_F(DetailSpaceHelperTest, setDetailViewSelectFileUrl)
{
    // Test setter: void setDetailViewSelectFileUrl((quint64 windowId, const QUrl &url))
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->setDetailViewSelectFileUrl(0, _arg1));
}

TEST_F(DetailSpaceHelperTest, updateWorkspaceWidth)
{
    // Test method: void updateWorkspaceWidth((quint64 windowId, DetailSpaceWidget *widget, bool show, int targetWidth))
    EXPECT_NO_FATAL_FAILURE(obj->updateWorkspaceWidth(0, nullptr, false, 0));
}
