// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>
#include <QUrl>
#include <QList>
#include "stubext.h"

#include "views/multifilepropertydialog.h"
#include "dfmplugin_propertydialog_global.h"

DPPROPERTYDIALOG_USE_NAMESPACE

class TestMultiFilePropertyDialog : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
    }

    void TearDown() override
    {
        stub.clear();
    }

    stub_ext::StubExt stub;
};

// Test MultiFilePropertyDialog class
TEST_F(TestMultiFilePropertyDialog, MultiFilePropertyDialogConstructor)
{
    QList<QUrl> urls;
    urls << QUrl::fromLocalFile("/tmp/test1.txt") << QUrl::fromLocalFile("/tmp/test2.txt");
    
    MultiFilePropertyDialog *dialog = new MultiFilePropertyDialog(urls);
    EXPECT_NE(dialog, nullptr);
    delete dialog;
}

TEST_F(TestMultiFilePropertyDialog, MultiFilePropertyDialogDestructor)
{
    QList<QUrl> urls;
    urls << QUrl::fromLocalFile("/tmp/test1.txt") << QUrl::fromLocalFile("/tmp/test2.txt");
    
    MultiFilePropertyDialog *dialog = new MultiFilePropertyDialog(urls);
    EXPECT_NO_THROW(delete dialog);
}

TEST_F(TestMultiFilePropertyDialog, MultiFilePropertyDialogUpdateFolderSizeLabel)
{
    QList<QUrl> urls;
    urls << QUrl::fromLocalFile("/tmp/test1.txt") << QUrl::fromLocalFile("/tmp/test2.txt");
    
    MultiFilePropertyDialog dialog(urls);
    EXPECT_NO_THROW(dialog.updateFolderSizeLabel(1024, 2, 1));
}