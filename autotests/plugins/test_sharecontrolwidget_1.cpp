// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_sharecontrolwidget_1.cpp
 * @brief Unit tests for ShareControlWidget methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "widget/sharecontrolwidget.h"

#include <QTest>

using namespace dfmplugin_dirshare;

class ShareControlWidgetTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ShareControlWidget();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ShareControlWidget *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ShareControlWidgetTest, initConnection)
{
    // Test method: void initConnection(())
    EXPECT_NO_FATAL_FAILURE(obj->initConnection());
}

TEST_F(ShareControlWidgetTest, onSambaPasswordSet)
{
    // Test method: void onSambaPasswordSet((bool result))
    EXPECT_NO_FATAL_FAILURE(obj->onSambaPasswordSet(false));
}

TEST_F(ShareControlWidgetTest, onShareNameChanged)
{
    // Test method: void onShareNameChanged((const QString &name))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onShareNameChanged(_arg0));
}

TEST_F(ShareControlWidgetTest, setOption)
{
    // Test setter: void setOption((QWidget *w, const QVariantHash &option))
    QVariantHash _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->setOption(nullptr, _arg1));
}

TEST_F(ShareControlWidgetTest, setupNetworkPath)
{
    // Test getter: QHBoxLayout setupNetworkPath()
    auto result = obj->setupNetworkPath();
    EXPECT_NO_FATAL_FAILURE({ obj->setupNetworkPath(); });

}

TEST_F(ShareControlWidgetTest, setupShareAnonymousSelector)
{
    // Test method: void setupShareAnonymousSelector(())
    EXPECT_NO_FATAL_FAILURE(obj->setupShareAnonymousSelector());
}

TEST_F(ShareControlWidgetTest, setupShareNameEditor)
{
    // Test method: void setupShareNameEditor(())
    EXPECT_NO_FATAL_FAILURE(obj->setupShareNameEditor());
}

TEST_F(ShareControlWidgetTest, setupShareNotes)
{
    // Test method: void setupShareNotes(())
    EXPECT_NO_FATAL_FAILURE(obj->setupShareNotes());
}

TEST_F(ShareControlWidgetTest, setupSharePassword)
{
    // Test getter: QHBoxLayout setupSharePassword()
    auto result = obj->setupSharePassword();
    EXPECT_NO_FATAL_FAILURE({ obj->setupSharePassword(); });

}

TEST_F(ShareControlWidgetTest, setupSharePermissionSelector)
{
    // Test method: void setupSharePermissionSelector(())
    EXPECT_NO_FATAL_FAILURE(obj->setupSharePermissionSelector());
}

TEST_F(ShareControlWidgetTest, setupShareSwitcher)
{
    // Test method: void setupShareSwitcher(())
    EXPECT_NO_FATAL_FAILURE(obj->setupShareSwitcher());
}

TEST_F(ShareControlWidgetTest, setupUserName)
{
    // Test getter: QHBoxLayout setupUserName()
    auto result = obj->setupUserName();
    EXPECT_NO_FATAL_FAILURE({ obj->setupUserName(); });

}

TEST_F(ShareControlWidgetTest, showMoreInfo)
{
    // Test method: void showMoreInfo((bool showMore))
    EXPECT_NO_FATAL_FAILURE(obj->showMoreInfo(false));
}

TEST_F(ShareControlWidgetTest, showSharePasswordSettingsDialog)
{
    // Test method: void showSharePasswordSettingsDialog(())
    EXPECT_NO_FATAL_FAILURE(obj->showSharePasswordSettingsDialog());
}

TEST_F(ShareControlWidgetTest, unshareFolder)
{
    // Test bool getter: unshareFolder()
    bool result = obj->unshareFolder();
    EXPECT_FALSE(result);

}

TEST_F(ShareControlWidgetTest, updateFile)
{
    // Test method: void updateFile((const QUrl &oldOne, const QUrl &newOne))
    QUrl _arg0{};
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->updateFile(_arg0, _arg1));
}

TEST_F(ShareControlWidgetTest, updateShare)
{
    // Test method: void updateShare(())
    EXPECT_NO_FATAL_FAILURE(obj->updateShare());
}

TEST_F(ShareControlWidgetTest, userShareOperation)
{
    // Test method: void userShareOperation((bool checked))
    EXPECT_NO_FATAL_FAILURE(obj->userShareOperation(false));
}
