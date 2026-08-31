// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_ddciiconpreview.cpp
 * @brief Unit tests for DDciIconPreview methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "apps/dde-file-manager-preview/pluginpreviews/dciicon-preview/ddciiconpreview.h"

#include <QTest>

using namespace src;

class DDciIconPreviewTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DDciIconPreview();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DDciIconPreview *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DDciIconPreviewTest, initControlWidgets)
{
    // Test method: void initControlWidgets(())
    EXPECT_NO_FATAL_FAILURE(obj->initControlWidgets());
}

TEST_F(DDciIconPreviewTest, setFileUrl)
{
    // Test method: bool setFileUrl((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->setFileUrl(_arg0);
    EXPECT_FALSE(result);

}
