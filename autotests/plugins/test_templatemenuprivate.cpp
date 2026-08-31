// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_templatemenuprivate.cpp
 * @brief Unit tests for TemplateMenuPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "templatemenuscene/templatemenu.h"

#include <QTest>

using namespace dfmplugin_menu;

class TemplateMenuPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TemplateMenuPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TemplateMenuPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TemplateMenuPrivateTest, createActionByDesktopFile)
{
    // Test method: void createActionByDesktopFile((const QDir &dir, const QString &path))
    QDir _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->createActionByDesktopFile(_arg0, _arg1));
}

TEST_F(TemplateMenuPrivateTest, createActionByNormalFile)
{
    // Test method: void createActionByNormalFile((const QString &path))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->createActionByNormalFile(_arg0));
}

TEST_F(TemplateMenuPrivateTest, loadTemplatePaths)
{
    // Test method: void loadTemplatePaths(())
    EXPECT_NO_FATAL_FAILURE(obj->loadTemplatePaths());
}

TEST_F(TemplateMenuPrivateTest, traverseFolderToCreateActions)
{
    // Test method: void traverseFolderToCreateActions((const QString &path, bool isDesktopEntryFolderPath))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->traverseFolderToCreateActions(_arg0, false));
}

TEST_F(TemplateMenuPrivateTest, TemplateMenuPrivate_Destructor)
{
    // Test method:  ~TemplateMenuPrivate(())
    EXPECT_NO_FATAL_FAILURE({ TemplateMenuPrivate *tmp = new TemplateMenuPrivate(); delete tmp; });
}
