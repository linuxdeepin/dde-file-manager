// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/core/dfmplugin-propertydialog/utils/propertydialogmanager.h"

#include <QUrl>

#include <gtest/gtest.h>

DPPROPERTYDIALOG_USE_NAMESPACE

class UT_PropertyDialogManager : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        mng = &PropertyDialogManager::instance();
    }
    virtual void TearDown() override { stub.clear(); }

private:
    stub_ext::StubExt stub;
    PropertyDialogManager *mng { nullptr };
};

TEST_F(UT_PropertyDialogManager, CreateCustomView)
{
    mng->viewCreateFunctionHash.clear();
    EXPECT_FALSE(mng->createCustomView(QUrl()));
    EXPECT_FALSE(mng->createCustomView(QUrl::fromLocalFile("/home")));

    mng->viewCreateFunctionHash.insert("entry", [](const QUrl &url) -> QWidget * {
        __DBG_STUB_INVOKE__
        return url.scheme() == "entry" ? new QWidget() : nullptr;
    });
    EXPECT_FALSE(mng->createCustomView(QUrl::fromLocalFile("/home")));
    QWidget *wid { nullptr };
    EXPECT_NO_FATAL_FAILURE(wid = mng->createCustomView(QUrl("entry:///sdb1.blockdev")));
    EXPECT_TRUE(wid);
    delete wid;
    mng->viewCreateFunctionHash.clear();
}
